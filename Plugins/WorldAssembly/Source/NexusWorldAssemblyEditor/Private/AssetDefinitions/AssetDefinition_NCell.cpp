// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "AssetDefinitions/AssetDefinition_NCell.h"
#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "Editor.h"
#include "EditorAssetLibrary.h"
#include "TimerManager.h"
#include "IAssetTools.h"
#include "Cell/NCellActor.h"
#include "NEditorDefaults.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "NWorldAssemblyEditorStyle.h"
#include "NWorldAssemblyEditorUtils.h"
#include "NWorldAssemblyUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetViewUtils.h"
#include "Cell/NTissue.h"
#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"
#include "Factories/DataAssetFactory.h"
#include "IContentBrowserSingleton.h"
#include "Misc/DataValidation.h"
#include "Misc/ScopedSlowTask.h"
#include "ToolMenus.h"
#include "UObject/ObjectSaveContext.h"

/**
 * Queue a side-car package for deletion on the next editor tick.
 *
 * Both OnAssetRemoved and OnAssetRenamed can fire while the engine is still unwinding an asset
 * rename/delete (a rename is internally a create-new + delete-old). UEditorAssetLibrary::DeleteAsset
 * runs CollectGarbage, so calling it synchronously from those broadcasts can free the very package
 * the engine's CleanupAfterSuccessfulDelete loop is mid-iteration over, leaving FUnsavedAssetsTracker
 * with a stale UPackage* that crashes in GetPathName. Deferring moves the delete (and its GC) out of
 * that call stack entirely.
 */
static void ScheduleSidecarDelete(const FString& SidecarPath)
{
	if (!FPackageName::DoesPackageExist(SidecarPath))
	{
		return;
	}

	GEditor->GetTimerManager()->SetTimerForNextTick(FTimerDelegate::CreateLambda([SidecarPath]()
	{
		if (FPackageName::DoesPackageExist(SidecarPath))
		{
			UEditorAssetLibrary::DeleteAsset(SidecarPath);
		}
	}));
}

/**
 * Side-cars dirtied by OnPreSaveWorldWithContext that still need their own SavePackage. The flush is deferred to the
 * matching OnPostSaveWorldWithContext so the write happens as a fresh top-level save, never re-entrantly inside the
 * host world's pre-save broadcast. Held as weak pointers so a side-car GC'd between pre- and post-save simply drops out.
 */
static TSet<TWeakObjectPtr<UNCell>> PendingSidecarFlushes;

/** @return the full object path of the side-car asset within the given side-car package. */
static FSoftObjectPath MakeSidecarObjectPath(const FString& SidecarPackage)
{
	// The side-car object name matches its package short name (see GetOrCreatePackage), so the
	// object path is "<Package>.<ShortName>".
	const FString AssetName = FPackageName::GetShortName(SidecarPackage);
	return FSoftObjectPath(FString::Printf(TEXT("%s.%s"), *SidecarPackage, *AssetName));
}

/** Pending old-cell -> new-cell side-car remaps accumulated across a rename batch. */
static TMap<FSoftObjectPath, FSoftObjectPath> PendingTissueRepoints;

/**
 * Scan every UNTissue once and apply all pending cell-reference remaps, saving changed tissues.
 *
 * The cell side-car is delete+recreated on rename rather than truly renamed, so no redirector exists
 * for UE's reference fixup to follow — tissue TSoftObjectPtr<UNCell> entries are left dangling. We scan
 * the tissues directly (rather than GetReferencers on the deleted side-car, whose dependency data the
 * registry may already have dropped) and rewrite the soft paths ourselves. Renaming several cells at
 * once queues multiple remaps that this single deferred pass resolves together.
 */
static void FlushTissueRepoints()
{
	const TMap<FSoftObjectPath, FSoftObjectPath> Remaps = MoveTemp(PendingTissueRepoints);
	PendingTissueRepoints.Reset();
	if (Remaps.IsEmpty())
	{
		return;
	}

	const IAssetRegistry& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	TArray<FAssetData> Tissues;
	AssetRegistry.GetAssetsByClass(UNTissue::StaticClass()->GetClassPathName(), Tissues);
	if (Tissues.IsEmpty())
	{
		return;
	}

	FScopedSlowTask SlowTask(static_cast<float>(Tissues.Num()),
		NSLOCTEXT("NexusWorldAssemblyEditor", "RepointTissues", "Updating tissue cell references..."));
	SlowTask.MakeDialog();

	for (const FAssetData& TissueData : Tissues)
	{
		SlowTask.EnterProgressFrame(1.0f, FText::Format(
			NSLOCTEXT("NexusWorldAssemblyEditor", "RepointTissuesItem", "Checking {0}"),
			FText::FromName(TissueData.AssetName)));

		UNTissue* Tissue = Cast<UNTissue>(TissueData.GetAsset());
		if (Tissue == nullptr)
		{
			continue;
		}

		bool bChanged = false;
		for (FNTissueEntry& Entry : Tissue->Cells)
		{
			if (const FSoftObjectPath* NewCell = Remaps.Find(Entry.Cell.ToSoftObjectPath()))
			{
				Entry.Cell = TSoftObjectPtr<UNCell>(*NewCell);
				bChanged = true;
			}
		}

		if (bChanged)
		{
			Tissue->MarkPackageDirty();
			UEditorAssetLibrary::SaveLoadedAsset(Tissue, false);
		}
	}
}

/**
 * Queue a cell remap and ensure a single deferred FlushTissueRepoints is scheduled for the batch.
 * The first call of a batch arms the next-tick flush; subsequent calls only add to the map, so
 * renaming N cells results in one tissue scan rather than N.
 */
static void QueueTissueRepoint(const FSoftObjectPath& OldCell, const FSoftObjectPath& NewCell)
{
	const bool bAlreadyScheduled = !PendingTissueRepoints.IsEmpty();
	PendingTissueRepoints.Add(OldCell, NewCell);
	if (!bAlreadyScheduled)
	{
		GEditor->GetTimerManager()->SetTimerForNextTick(FTimerDelegate::CreateLambda(&FlushTissueRepoints));
	}
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_NCell::GetAssetCategories() const
{
	static const auto Categories = { FNEditorDefaults::AssetCategory };
	return Categories;
}

FText UAssetDefinition_NCell::GetAssetDescription(const FAssetData& AssetData) const
{
	if (const UNCell* Asset = Cast<UNCell>(AssetData.GetAsset()))
	{
		return FText::FromString(FString::Printf(TEXT("%s"), *Asset->World.GetAssetName()));
	}
	return FText::GetEmpty();
}

/** Jump the Content Browser to the level the selected NCell describes. */
static void SelectLevelForCell(const FToolMenuContext& InContext)
{
	const UContentBrowserAssetContextMenuContext* Context = UContentBrowserAssetContextMenuContext::FindContextWithAssets(InContext);
	if (Context == nullptr || Context->SelectedAssets.IsEmpty()) return;

	const UNCell* Cell = Cast<UNCell>(Context->SelectedAssets[0].GetAsset());
	if (Cell == nullptr || Cell->World.IsNull()) return;

	const IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	const FAssetData WorldAssetData = AssetRegistry.GetAssetByObjectPath(Cell->World.ToSoftObjectPath());
	if (!WorldAssetData.IsValid()) return;

	FContentBrowserModule& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowser.Get().SyncBrowserToAssets({ WorldAssetData });
}

/**
 * Register the NCell asset context-menu entries.
 *
 * @note Where "Select Level in Content Browser" lives now. It used to be the click action on a thumbnail overlay,
 *       which had to be abandoned: FAssetActionThumbnailOverlayInfo carries a single ActionImageWidget that
 *       SAssetThumbnail parents into two slots at once — the engine's own comment there calls it out as wrong and
 *       kept only for API compatibility — and a widget with a mismatched parent fails
 *       SWidget::SupportsInvalidationRecursive, which disables caching for the whole Content Browser asset view.
 *       The context menu is what UAssetDefinition's own documentation points at for asset actions anyway.
 */
static FDelayedAutoRegisterHelper RegisterNCellAssetMenu(EDelayedRegisterRunPhase::EndOfEngineInit, []
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
	{
		FToolMenuOwnerScoped OwnerScoped(UE_MODULE_NAME);

		UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UNCell::StaticClass());
		if (Menu == nullptr) return;

		FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");
		Section.AddDynamicEntry(NAME_None, FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
		{
			const UContentBrowserAssetContextMenuContext* Context = UContentBrowserAssetContextMenuContext::FindContextWithAssets(InSection);
			if (Context == nullptr || Context->SelectedAssets.IsEmpty()) return;

			FToolUIAction UIAction;
			UIAction.ExecuteAction = FToolMenuExecuteAction::CreateStatic(&SelectLevelForCell);

			InSection.AddMenuEntry("NCell_SelectLevel",
				NSLOCTEXT("NexusWorldAssemblyEditor", "NCell_SelectInContentBrowser", "Select Level in Content Browser"),
				NSLOCTEXT("NexusWorldAssemblyEditor", "NCell_SelectInContentBrowserTooltip", "Jump the Content Browser to the level this NCell describes."),
				FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "ClassIcon.NCellLevelInstance"),
				UIAction);
		}));
	}));
});

FText UAssetDefinition_NCell::GetAssetDisplayName() const
{
	static const FText DisplayName = NSLOCTEXT("NexusWorldAssemblyEditor", "AssetTypeActions_NCell", "NCell");
	return DisplayName;
}

UNCell* UAssetDefinition_NCell::GetOrCreatePackage(UWorld* World)
{
	const FString FullName = GetCellPackagePath(World->GetOutermost()->GetName());
	const FString ShortName = FPackageName::GetShortName(FullName);
	const FString PackagePath = FPaths::GetPath(FullName);

	// Load already existing object and return it
	if (FPackageName::DoesPackageExist(FullName))
	{
		return LoadObject<UNCell>(nullptr, *FullName);
	}

	UDataAssetFactory* Factory = NewObject<UDataAssetFactory>();

	// Create Asset
	IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UNCell* Asset = Cast<UNCell>(AssetTools.CreateAsset(ShortName, *PackagePath, UNCell::StaticClass(), Factory));
	if (Asset == nullptr)
	{
		UE_LOG(LogNexusWorldAssemblyEditor, Warning, TEXT("Failed to create UNCell side-car package at %s."), *FullName);
		return nullptr;
	}

	Asset->World = TSoftObjectPtr<UWorld>(World);

	// Write to disk first
	UEditorAssetLibrary::SaveLoadedAsset(Asset, false);

	FAssetRegistryModule::AssetCreated(Asset);
	return Asset;
}

void UAssetDefinition_NCell::OnAssetRemoved(const FAssetData& AssetData)
{
	if (!AssetData.IsValid() ||
		AssetData.AssetClassPath != UWorld::StaticClass()->GetClassPathName())
	{
		return;
	}

	ScheduleSidecarDelete(GetCellPackagePath(AssetData.PackageName.ToString()));
}

void UAssetDefinition_NCell::OnAssetRenamed(const FAssetData& AssetData, const FString& String)
{
	if (!AssetData.IsValid() ||
		AssetData.AssetClassPath != UWorld::StaticClass()->GetClassPathName())
	{
		return;
	}

	// String is the old object path; resolve it back to the old package so the side-car path is
	// mount-point-qualified (e.g. /Game/Cells/CELL_Foo_NCell) rather than a bare short name. This
	// mirrors how OnAssetRemoved derives the path from the full package name.
	const FString OldPackage = FPackageName::ObjectPathToPackageName(String);
	const FString OldSidecarPackage = GetCellPackagePath(OldPackage);
	const FString NewSidecarPackage = GetCellPackagePath(AssetData.PackageName.ToString());

	// Queue a tissue cell-reference remap from the old side-car path to the new one. Deferred for the
	// same reason as the delete: saving packages from inside the rename broadcast is unsafe. Renaming
	// several cells at once coalesces into a single tissue scan via FlushTissueRepoints.
	QueueTissueRepoint(MakeSidecarObjectPath(OldSidecarPackage), MakeSidecarObjectPath(NewSidecarPackage));

	// Delete the old side-car (a new one is created during the resave under the new name). Deferred —
	// see ScheduleSidecarDelete for why this must not run synchronously inside the rename broadcast.
	ScheduleSidecarDelete(OldSidecarPackage);
}

void UAssetDefinition_NCell::OnPreSaveWorldWithContext(UWorld* World, FObjectPreSaveContext ObjectPreSaveContext)
{
	// Sidecar creation/sync is editor-only authoring; never run it during a cook save of the level.
	// Doing so loads/creates the paired UNCell mid-cook, which the cooker flags as an unexpected load.
	if (ObjectPreSaveContext.IsCooking())
	{
		return;
	}

	ANCellActor* CellActor = FNWorldAssemblyUtils::GetCellActorFromWorld(World, true);

	// We don't have a cell actor, get out of here!
	if (CellActor == nullptr) return;

	// Ensure CellActor settings
	FNWorldAssemblyEditorUtils::EnsureCellInitializedCallbackActors(World, CellActor);

	// Sync the cell data into its side-car now (in-memory), so the recalculated actor state is captured by this level
	// save, but defer the side-car's own disk write to OnPostSaveWorldWithContext rather than running a re-entrant
	// SavePackage inside this pre-save broadcast.
	if (UNCell* Cell = FNWorldAssemblyEditorUtils::SyncCell(World, CellActor))
	{
		PendingSidecarFlushes.Add(Cell);
	}
}

void UAssetDefinition_NCell::OnPostSaveWorldWithContext(UWorld* World, FObjectPostSaveContext ObjectPostSaveContext)
{
	// Nothing is queued during a cook save (OnPreSaveWorldWithContext bails first); mirror the guard for symmetry.
	if (ObjectPostSaveContext.IsCooking())
	{
		return;
	}

	const ANCellActor* CellActor = FNWorldAssemblyUtils::GetCellActorFromWorld(World, true);
	if (CellActor == nullptr) return;

	// Flush this world's side-car if its pre-save pass dirtied it. The world package is fully written by now, so this is
	// a safe top-level save; SaveLoadedAsset routes through SavePackages, which checks the package out (p4 edit) or marks
	// a freshly created one for add (p4 add) under source control. Only dequeue on success so a failed checkout (offline,
	// exclusively locked) retries on the next save rather than being silently dropped.
	UNCell* Cell = CellActor->Sidecar.Get();
	if (Cell != nullptr && PendingSidecarFlushes.Contains(Cell))
	{
		if (UEditorAssetLibrary::SaveLoadedAsset(Cell))
		{
			PendingSidecarFlushes.Remove(Cell);
		}
		else
		{
			// Kept queued for a retry on the next save. Surface it so a source-control failure (not checked out,
			// exclusively locked, offline) isn't silently swallowed mid level-save — the data is safe in memory.
			UE_LOG(LogNexusWorldAssemblyEditor, Warning,
				TEXT("Failed to write the UNCell side-car '%s' to disk after saving world '%s'. It may not be checked out in source control; the change is kept in memory and will be retried on the next save."),
				*Cell->GetName(), *World->GetName());
		}
	}
}

EDataValidationResult UAssetDefinition_NCell::ValidateAsset(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{

	const UNCell* Cell = Cast<UNCell>(InAsset);
	if (!Cell) return EDataValidationResult::NotValidated;
	EDataValidationResult Result = EDataValidationResult::Valid;

	if (Cell->World.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::Format(NSLOCTEXT("NexusWorldAssemblyEditor", "Validate_NCell_MissingWorld", "Cell {0} has no World set."), FText::FromString(Cell->GetName())));
	}

	if (Cell->Root.Bounds.GetSize() == FVector::ZeroVector)
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::Format(NSLOCTEXT("NexusWorldAssemblyEditor", "Validate_NCell_BoundsSizeZero", "Cell {0} has no bounds."), FText::FromString(Cell->GetName())));
	}

	if (Cell->Root.Hull.Vertices.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::Format(NSLOCTEXT("NexusWorldAssemblyEditor", "Validate_NCell_HullNoVertices", "Cell {0} hull has no vertices."), FText::FromString(Cell->GetName())));
	}

	if (!Cell->Root.HullSettings.bAllowNonConvex && !Cell->Root.Hull.IsConvex())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::Format(NSLOCTEXT("NexusWorldAssemblyEditor", "Validate_NCell_HullNotConvex", "Cell {0} hull is not convex, and not allowed."), FText::FromString(Cell->GetName())));
	}

	if (Cell->Junctions.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::Format(NSLOCTEXT("NexusWorldAssemblyEditor", "Validate_NCell_NoJunctions", "Cell {0} has no junctions."), FText::FromString(Cell->GetName())));
	}

	return Result;
}

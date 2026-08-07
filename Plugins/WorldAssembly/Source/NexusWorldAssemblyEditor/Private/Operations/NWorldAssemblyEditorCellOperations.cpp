// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Operations/NWorldAssemblyEditorCellOperations.h"

#include "AssetViewUtils.h"
#include "EditorAssetLibrary.h"
#include "FileHelpers.h"
#include "ImageUtils.h"
#include "LevelEditorViewport.h"
#include "ObjectTools.h"
#include "ScopedTransaction.h"
#include "AssetDefinitions/AssetDefinition_NCell.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Cell/NCell.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellRootDetails.h"
#include "Cell/NCellVoxelData.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/MessageDialog.h"
#include "Misc/ObjectThumbnail.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "NWorldAssemblyEditorSettings.h"
#include "NWorldAssemblyEditorUtils.h"
#include "NWorldAssemblyUtils.h"

void FNWorldAssemblyEditorCellOperations::AddActor()
{
	UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld == nullptr)
	{
		return;
	}

	if (FNEditorUtils::IsUnsavedWorld(CurrentWorld))
	{
		const EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Type::YesNo,
			FText::FromString(TEXT("You need to save the world/map that you are working in before creating a NCellActor.\n\nDo you wish to save the map now?")),
			FText::FromString(TEXT("NEXUS: World Assembly")));
		switch (Choice)
		{
		case EAppReturnType::No:
			UE_LOG(LogNexusWorldAssemblyEditor, Error, TEXT("Unable to add UNCellActor to an unsaved world."));
			return;
		case EAppReturnType::Yes:
			if (!FEditorFileUtils::SaveLevel(CurrentWorld->GetCurrentLevel()))
			{
				UE_LOG(LogNexusWorldAssemblyEditor, Error, TEXT("Unable to add UNCellActor to an unsaved world."));
				return;
			}
			break;
		default:
			UE_LOG(LogNexusWorldAssemblyEditor, Error, TEXT("Unable to add UNCellActor to an unsaved world."));
			return;
		}
	}

	ANCellActor* SpawnedActor = CurrentWorld->SpawnActor<ANCellActor>(ANCellActor::StaticClass(), FTransform::Identity, FActorSpawnParameters());
	if (SpawnedActor == nullptr)
	{
		UE_LOG(LogNexusWorldAssemblyEditor, Warning, TEXT("Failed to spawn the UNCellActor in the current world."));
		return;
	}

	// Apply default settings to the Cell
	const UNWorldAssemblyEditorSettings* Settings = UNWorldAssemblyEditorSettings::Get();
	Settings->ApplyDefaultSettings(SpawnedActor->GetCellRoot());

	FNWorldAssemblyEditorUtils::SaveCell(CurrentWorld, SpawnedActor);
}

void FNWorldAssemblyEditorCellOperations::RemoveActor()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCellOperations_RemoveActor", "Remove Cell Actor"));
	UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	ANCellActor* CellActor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
	if (!ensure(CellActor != nullptr)) return;
	CellActor->Destroy();
	// ReSharper disable once CppExpressionWithoutSideEffects
	CurrentWorld->MarkPackageDirty();

	// Need to destroy the sidecar.
	if (const FString SidecarPath = UAssetDefinition_NCell::GetCellPackagePath(CurrentWorld->GetOutermost()->GetName());
		FPackageName::DoesPackageExist(SidecarPath))
		{
			UEditorAssetLibrary::DeleteAsset(SidecarPath);
		}
}

void FNWorldAssemblyEditorCellOperations::ResetCell()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCellOperations_ResetCell", "Reset Cell"));
	// Get the cell actor
	ANCellActor* CellActor = FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld();
	if (!ensure(CellActor != nullptr)) return;
	CellActor->Modify();

	// Apply default settings to the Cell
	const UNWorldAssemblyEditorSettings* Settings = UNWorldAssemblyEditorSettings::Get();
	Settings->ApplyDefaultSettings(CellActor->GetCellRoot());

	// Get the cell
	UNCell* Cell = UAssetDefinition_NCell::GetOrCreatePackage(FNEditorUtils::GetCurrentWorld());
	if (Cell == nullptr)
	{
		UE_LOG(LogNexusWorldAssemblyEditor, Warning, TEXT("Unable to get or create the UNCell side-car package when trying to reset the cell."));
		return;
	}
	Cell->Root = FNCellRootDetails();
	Cell->Junctions.Empty();

	// Update the data
	FNWorldAssemblyEditorUtils::UpdateCell(Cell, CellActor);

	// Flag the actor as dirty
	CellActor->SetActorDirty();
}

void FNWorldAssemblyEditorCellOperations::CalculateAll()
{
	CalculateBounds();
	CalculateHull();
	CalculateVoxelData();
}

void FNWorldAssemblyEditorCellOperations::CalculateBounds()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCellOperations_CalculateBounds", "Calculate Cell Bounds"));
	ANCellActor* CellActor = FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld();
	if (!ensure(CellActor != nullptr)) return;
	CellActor->CalculateBounds();
}

void FNWorldAssemblyEditorCellOperations::CalculateHull()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCellOperations_CalculateHull", "Calculate Cell Hull"));
	ANCellActor* CellActor = FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld();
	if (!ensure(CellActor != nullptr)) return;
	CellActor->CalculateHull();

	UNWorldAssemblyEdMode::ProtectCellEdMode();
}

void FNWorldAssemblyEditorCellOperations::CalculateVoxelData()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCellOperations_CalculateVoxel", "Calculate Voxel Data"));
	ANCellActor* CellActor = FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld();
	if (!ensure(CellActor != nullptr)) return;
	CellActor->CalculateVoxelData();
}

void FNWorldAssemblyEditorCellOperations::ModifyCellRootDetails(const FText& TransactionName, TFunctionRef<void(FNCellRootDetails&)> Mutator)
{
	const FScopedTransaction Transaction(TransactionName);

	const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld == nullptr) return;

	ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
	if (Actor == nullptr) return;

	Actor->Modify();
	Mutator(Actor->GetCellRoot()->Details);
	Actor->SetActorDirty();
}

bool FNWorldAssemblyEditorCellOperations::QueryCellRootDetails(TFunctionRef<bool(const FNCellRootDetails&)> Reader)
{
	const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld == nullptr) return false;

	const ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
	if (Actor == nullptr) return false;

	return Reader(Actor->GetCellRoot()->Details);
}

/** Edge length of the badge composited into a captured NCell thumbnail, in thumbnail pixels. */
static constexpr int32 CellThumbnailBadgeSize = 48;

/** Gap left between the badge and the top-left corner of the thumbnail, in thumbnail pixels. */
static constexpr int32 CellThumbnailBadgeMargin = 8;

/**
 * Blend the NCell badge into the top-left corner of an asset's freshly captured thumbnail.
 *
 * @param AssetData The asset whose cached thumbnail to stamp.
 * @note Baked into the image rather than drawn as a Content Browser overlay. The overlay path this replaces handed
 *       SAssetThumbnail one widget that it parented into two slots at once, which fails
 *       SWidget::SupportsInvalidationRecursive and disabled caching for the whole asset view. Baking also puts the
 *       badge on every surface that shows a thumbnail — asset pickers, the reference viewer — not just tiles.
 * @remark Only affects thumbnails captured from here on. An NCell whose thumbnail predates this, or that has none,
 *         goes unbadged until it is captured again.
 */
static void ApplyCellThumbnailBadge(const FAssetData& AssetData)
{
	UObject* Asset = AssetData.GetAsset();
	if (Asset == nullptr) return;

	FObjectThumbnail* Thumbnail = ThumbnailTools::GetThumbnailForObject(Asset);
	if (Thumbnail == nullptr) return;

	const int32 Width = Thumbnail->GetImageWidth();
	const int32 Height = Thumbnail->GetImageHeight();

	// Only when the pixels are genuinely absent. DecompressImageData resets ImageData before consulting the
	// compressor, and the compressor is null while CompressedImageData is empty — which is exactly the state
	// CaptureThumbnailFromViewport leaves behind, since it fills ImageData and never compresses. Calling it
	// unconditionally throws the freshly captured pixels away and puts nothing back.
	if (Thumbnail->AccessImageData().IsEmpty())
	{
		Thumbnail->DecompressImageData();
	}

	TArray<uint8>& Pixels = Thumbnail->AccessImageData();
	if (Width <= 0 || Height <= 0 || Pixels.Num() < Width * Height * 4) return;

	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("NexusWorldAssembly"));
	if (!Plugin.IsValid()) return;

	const FString BadgePath = Plugin->GetBaseDir() / TEXT("Resources") / TEXT("AssetOverlay_NCell.png");

	FImage Badge;
	if (!FImageUtils::LoadImage(*BadgePath, Badge)) return;

	// ResizeTo rather than FImageUtils::ImageResize: that one defaults to forcing an opaque result, which would
	// discard the alpha this needs to blend the logo over the square below.
	//
	// The margin comes out of the space available to the badge, so a thumbnail too small to hold both still gets
	// a proportionally inset badge rather than one clipped off its own edge.
	const int32 Margin = FMath::Min3(CellThumbnailBadgeMargin, Width / 2, Height / 2);
	const int32 BadgeSize = FMath::Min3(CellThumbnailBadgeSize, Width - Margin, Height - Margin);
	if (BadgeSize <= 0) return;

	FImage ScaledBadge;
	Badge.ResizeTo(ScaledBadge, BadgeSize, BadgeSize, ERawImageFormat::BGRA8, EGammaSpace::sRGB);

	const TArrayView64<const FColor> BadgePixels = ScaledBadge.AsBGRA8();
	if (BadgePixels.Num() < static_cast<int64>(BadgeSize) * BadgeSize) return;

	// The thumbnail buffer is BGRA8, matching the format the badge was just converted into.
	FColor* ThumbnailPixels = reinterpret_cast<FColor*>(Pixels.GetData());

	for (int32 Y = 0; Y < BadgeSize; Y++)
	{
		for (int32 X = 0; X < BadgeSize; X++)
		{
			const FColor& Source = BadgePixels[static_cast<int64>(Y) * BadgeSize + X];
			FColor& Destination = ThumbnailPixels[(Y + Margin) * Width + (X + Margin)];

			// The square first, so the logo reads against the captured viewport whatever happened to be behind it.
			Destination = FColor::Black;

			if (Source.A == 0) continue;

			// Straight alpha over the square. Kept opaque: a thumbnail has no transparency to preserve.
			const int32 Alpha = Source.A;
			Destination.R = static_cast<uint8>((Source.R * Alpha) / 255);
			Destination.G = static_cast<uint8>((Source.G * Alpha) / 255);
			Destination.B = static_cast<uint8>((Source.B * Alpha) / 255);
			Destination.A = 255;
		}
	}

	// Refresh the compressed copy, which is what actually gets written to the package — leaving the pre-badge one
	// in place would discard every pixel above on save.
	Thumbnail->CompressImageData();
	Thumbnail->MarkAsDirty();

	if (UPackage* Package = Asset->GetOutermost())
	{
		Package->MarkPackageDirty();
	}
}

void FNWorldAssemblyEditorCellOperations::CaptureThumbnail()
{
	FViewport* Viewport = GEditor->GetActiveViewport();
	if (Viewport != nullptr &&
		ensure(GCurrentLevelEditingViewportClient) && ensure(Viewport) )
	{
		FLevelEditorViewportClient* OldViewportClient = GCurrentLevelEditingViewportClient;
		GCurrentLevelEditingViewportClient = nullptr;
		TArray<FAssetData> SelectedAssets;
		const FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		// Level
		const UWorld* World = FNEditorUtils::GetCurrentWorld();
		if (World != nullptr)
		{
			FAssetData LevelAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(World));
			SelectedAssets.Emplace(LevelAssetData);
		}
		if (SelectedAssets.Num() > 0)
		{
			UNWorldAssemblyEdMode::SetRenderMode(ENWorldAssemblyEdModeRenderMode::LevelScreenshot);
			Viewport->Draw();
			AssetViewUtils::CaptureThumbnailFromViewport(Viewport, SelectedAssets);
		}

		// Cell Data
		SelectedAssets.Empty();
		ANCellActor* CellActor = FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld();
		if (CellActor != nullptr)
		{
			FAssetData CellAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(CellActor->Sidecar.ToSoftObjectPath()));
			SelectedAssets.Emplace(CellAssetData);
		}
		if (SelectedAssets.Num() > 0)
		{
			UNWorldAssemblyEdMode::SetRenderMode(ENWorldAssemblyEdModeRenderMode::CellScreenshot);
			Viewport->Draw();
			AssetViewUtils::CaptureThumbnailFromViewport(Viewport, SelectedAssets);

			// Only the sidecar gets the badge. The level captured above is an ordinary world asset and has no reason
			// to advertise itself as an NCell.
			for (const FAssetData& CellAssetData : SelectedAssets)
			{
				ApplyCellThumbnailBadge(CellAssetData);
			}
		}

		GCurrentLevelEditingViewportClient = OldViewportClient;
		UNWorldAssemblyEdMode::SetRenderMode(ENWorldAssemblyEdModeRenderMode::All);
		Viewport->Draw();
	}
}

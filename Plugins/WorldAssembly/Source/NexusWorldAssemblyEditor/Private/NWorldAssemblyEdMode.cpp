// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEdMode.h"

#include "Editor.h"
#include "NActorUtils.h"
#include "NCanvasUtils.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"
#include "NWorldAssemblyRegistry.h"
#include "Cell/NCellRootComponent.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEditorSettings.h"
#include "NWorldAssemblyEditorUserSettings.h"
#include "Developer/NPrimitiveFont.h"
#include "NWorldAssemblyEditorUtils.h"
#include "NWorldAssemblySettings.h"
#include "NWorldAssemblyUtils.h"
#include "Assembly/Tasks/NCreateVirtualWorldTask.h"
#include "Macros/NFlagsMacros.h"
#include "Math/NBoxUtils.h"
#include "Math/NVectorUtils.h"
#include "Assembly/NAssemblyOperation.h"
#include "Developer/NMethodScopeTimer.h"
#include "UObject/UnrealType.h"

void FNWorldAssemblyEdMode::ProtectCellEdMode()
{
	if (ANCellActor* Actor = CellActor.Get();
		Actor != nullptr && Actor->GetCellRoot()->Details.Hull.HasNonTris() &&
		GetCellEdMode() == ENCellEdMode::Hull)
	{
		SetCellEdMode(ENCellEdMode::Bounds);
	}
}

void FNWorldAssemblyEdMode::OnActorDeleted(AActor* Actor)
{
	if (Actor == CellActor.Get())
	{
		CellActor.Reset();
	}
	if (Actor == CollisionVisualizer)
	{
		// The visualizer itself was removed (e.g. deleted by the user) — stop listening and clear our state.
		UnbindWorldChangeDelegates();
		CollisionSourceActors.Reset();
		bCollisionVisualizerDirty = false;
		CollisionVisualizer = nullptr;
	}
	else if (CollisionVisualizer != nullptr && CollisionSourceActors.Contains(FObjectKey(Actor)))
	{
		// A source actor was deleted; it can no longer pass the live filter (it's pending kill), so the source-set
		// membership is what tells us the visualizer needs rebuilding.
		MarkCollisionVisualizerDirty();
	}
}

TObjectPtr<ANDebugActor> FNWorldAssemblyEdMode::CreateCollisionVisualizer(UWorld* World)
{
	const bool bWasAlive = CollisionVisualizer != nullptr;

	// Only time the initial build; in-place refreshes are frequent and would otherwise spam the log.
	TOptional<FNMethodScopeTimer> Timer;
	if (!bWasAlive)
	{
		Timer.Emplace(TEXT("World Collision Build Time"));
	}

	TArray<AActor*> SourceActors;
	CollisionVisualizer = FNWorldAssemblyEditorUtils::RefreshWorldCollisionVisualizerActor(
		World, TArray<FBoxSphereBounds>(), CollisionVisualizer, SourceActors);

	bCollisionVisualizerDirty = false;

	if (CollisionVisualizer == nullptr)
	{
		// Nothing was spawned (no geometry / no material) — nothing to track or listen for.
		CollisionSourceActors.Reset();
		return nullptr;
	}

	// Refresh the source set used to test relevance of future world changes.
	CollisionSourceActors.Reset();
	CollisionSourceActors.Reserve(SourceActors.Num());
	for (const AActor* SourceActor : SourceActors)
	{
		CollisionSourceActors.Add(FObjectKey(SourceActor));
	}

	// Start listening only once a visualizer is actually alive.
	if (!bWasAlive)
	{
		BindWorldChangeDelegates();
	}

	return CollisionVisualizer;
}

void FNWorldAssemblyEdMode::DestroyCollisionVisualizer()
{
	UnbindWorldChangeDelegates();
	CollisionSourceActors.Reset();
	bCollisionVisualizerDirty = false;

	if (CollisionVisualizer != nullptr)
	{
		if (CollisionVisualizer->IsSelected())
		{
			GEditor->SelectActor(CollisionVisualizer, false, false);
		}
		CollisionVisualizer->GetWorld()->DestroyActor(CollisionVisualizer, false, false);
		CollisionVisualizer = nullptr;
	}
}

void FNWorldAssemblyEdMode::BindWorldChangeDelegates()
{
	if (!OnLevelActorAddedHandle.IsValid())
	{
		OnLevelActorAddedHandle = GEngine->OnLevelActorAdded().AddStatic(&FNWorldAssemblyEdMode::OnLevelActorAdded);
	}
	if (!OnObjectMovedHandle.IsValid())
	{
		OnObjectMovedHandle = GEditor->OnEndObjectMovement().AddStatic(&FNWorldAssemblyEdMode::OnObjectMoved);
	}
	if (!OnObjectPropertyChangedHandle.IsValid())
	{
		OnObjectPropertyChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddStatic(&FNWorldAssemblyEdMode::OnObjectPropertyChanged);
	}
	if (!OnUndoRedoHandle.IsValid())
	{
		OnUndoRedoHandle = FEditorDelegates::PostUndoRedo.AddStatic(&FNWorldAssemblyEdMode::OnUndoRedo);
	}
}

void FNWorldAssemblyEdMode::UnbindWorldChangeDelegates()
{
	if (OnLevelActorAddedHandle.IsValid())
	{
		GEngine->OnLevelActorAdded().Remove(OnLevelActorAddedHandle);
		OnLevelActorAddedHandle.Reset();
	}
	if (OnObjectMovedHandle.IsValid())
	{
		GEditor->OnEndObjectMovement().Remove(OnObjectMovedHandle);
		OnObjectMovedHandle.Reset();
	}
	if (OnObjectPropertyChangedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(OnObjectPropertyChangedHandle);
		OnObjectPropertyChangedHandle.Reset();
	}
	if (OnUndoRedoHandle.IsValid())
	{
		FEditorDelegates::PostUndoRedo.Remove(OnUndoRedoHandle);
		OnUndoRedoHandle.Reset();
	}
}

bool FNWorldAssemblyEdMode::ShouldRebuildForActor(const AActor* Actor)
{
	if (Actor == nullptr || CollisionVisualizer == nullptr) return false;

	// Was it part of the geometry we last built? (covers delete / collision-off / ignore-tag-added transitions)
	if (CollisionSourceActors.Contains(FObjectKey(Actor))) return true;

	// Is it relevant now? (covers add / collision-on transitions) — same predicate the visualizer build uses.
	return FNActorUtils::PassesFilter(Actor, FNCreateVirtualWorldTask::CreateWorldActorFilterSettings(UNWorldAssemblySettings::Get()->WorldCollisionSettings));
}

AActor* FNWorldAssemblyEdMode::ResolveAffectedActor(UObject* Object)
{
	if (Object == nullptr) return nullptr;
	if (AActor* Actor = Cast<AActor>(Object)) return Actor;
	if (const UActorComponent* Component = Cast<UActorComponent>(Object)) return Component->GetOwner();
	return nullptr;
}

void FNWorldAssemblyEdMode::OnLevelActorAdded(AActor* Actor)
{
	if (ShouldRebuildForActor(Actor))
	{
		MarkCollisionVisualizerDirty();
	}
}

void FNWorldAssemblyEdMode::OnObjectMoved(UObject& Object)
{
	if (ShouldRebuildForActor(ResolveAffectedActor(&Object)))
	{
		MarkCollisionVisualizerDirty();
	}
}

void FNWorldAssemblyEdMode::OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	// Ignore the continuous mid-edit stream (slider scrubs, gizmo drags); we rebuild on the finalizing change instead.
	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive) return;

	if (ShouldRebuildForActor(ResolveAffectedActor(Object)))
	{
		MarkCollisionVisualizerDirty();
	}
}

void FNWorldAssemblyEdMode::OnUndoRedo()
{
	if (CollisionVisualizer != nullptr)
	{
		MarkCollisionVisualizerDirty();
	}
}

void FNWorldAssemblyEdMode::CacheUserSettings()
{
	const UNWorldAssemblyEditorUserSettings* Settings = UNWorldAssemblyEditorUserSettings::Get();

	CachedCellHullColor = Settings->ColorPaletteCellHull;
	CachedCellBoundsColor = Settings->ColorPaletteCellBounds;

	CachedJunctionUnfilledColor = Settings->ColorPaletteJunctionsUnfilled;
	CachedJunctionValidColor = Settings->ColorPaletteJunctionsValid;
	CachedJunctionInvalidColor = Settings->ColorPaletteJunctionsInvalid;
}

const FEditorModeID FNWorldAssemblyEdMode::Identifier = TEXT("NWorldAssemblyEdMode");
const FText FNWorldAssemblyEdMode::DirtyMessage = FText::FromString("Dirty Cell Actor");
const FText FNWorldAssemblyEdMode::AutoBoundsMessage = FText::FromString("Cell Bounds not calculated on save.");
const FText FNWorldAssemblyEdMode::AutoBoundsHullMessage = FText::FromString("Cell Bounds and Hull not calculated on save.");
const FText FNWorldAssemblyEdMode::AutoHullMessage = FText::FromString("Cell Hull not calculated on save.");
const FText FNWorldAssemblyEdMode::AutoVoxelMessage = FText::FromString("Cell Voxel not calculated on save.");

TWeakObjectPtr<ANCellActor> FNWorldAssemblyEdMode::CellActor = nullptr;
FNWorldAssemblyEdMode::ENCellEdMode FNWorldAssemblyEdMode::CellEdMode = ENCellEdMode::Bounds;
TArray<FVector> FNWorldAssemblyEdMode::CachedHullVertices;
TArray<FIntVector2> FNWorldAssemblyEdMode::CachedHullEdges;

FBox FNWorldAssemblyEdMode::CachedBounds;
FNCellVoxelData FNWorldAssemblyEdMode::CachedVoxelData;

FLinearColor FNWorldAssemblyEdMode::CachedCellHullColor = NEXUS::WorldAssembly::DefaultColors::CellHull;
FLinearColor FNWorldAssemblyEdMode::CachedCellBoundsColor = NEXUS::WorldAssembly::DefaultColors::CellBounds;
FLinearColor FNWorldAssemblyEdMode::CachedJunctionUnfilledColor = NEXUS::WorldAssembly::DefaultColors::JunctionUnfilled;
FLinearColor FNWorldAssemblyEdMode::CachedJunctionInvalidColor = NEXUS::WorldAssembly::DefaultColors::JunctionInvalid;
FLinearColor FNWorldAssemblyEdMode::CachedJunctionValidColor = NEXUS::WorldAssembly::DefaultColors::JunctionValid;
FLinearColor FNWorldAssemblyEdMode::CachedBoneValidColor = NEXUS::WorldAssembly::DefaultColors::BoneValid;
FLinearColor FNWorldAssemblyEdMode::CachedBoneInvalidColor = NEXUS::WorldAssembly::DefaultColors::BoneInvalid;

TArray<FVector> FNWorldAssemblyEdMode::CachedBoundsVertices;
FNWorldAssemblyEdMode::ENCellVoxelMode FNWorldAssemblyEdMode::CellVoxelMode = ENCellVoxelMode::None;
TObjectPtr<ANDebugActor> FNWorldAssemblyEdMode::CollisionVisualizer = nullptr;
ENWorldAssemblyEdModeRenderMode FNWorldAssemblyEdMode::RenderMode = ENWorldAssemblyEdModeRenderMode::All;
TSet<FObjectKey> FNWorldAssemblyEdMode::CollisionSourceActors;
bool FNWorldAssemblyEdMode::bCollisionVisualizerDirty = false;
FDelegateHandle FNWorldAssemblyEdMode::OnLevelActorAddedHandle;
FDelegateHandle FNWorldAssemblyEdMode::OnObjectMovedHandle;
FDelegateHandle FNWorldAssemblyEdMode::OnObjectPropertyChangedHandle;
FDelegateHandle FNWorldAssemblyEdMode::OnUndoRedoHandle;

FNWorldAssemblyEdMode::~FNWorldAssemblyEdMode()
{
	if (OrganGenerator != nullptr)
	{
		OrganGenerator->TearDownOperation();
		OrganGenerator = nullptr;
	}
}

void FNWorldAssemblyEdMode::Enter()
{
	// Initialize cached values
	CellActor = nullptr;
	CachedHullVertices.Empty();
	CachedHullEdges.Empty();
	CachedBounds = FBox(ForceInit);
	CachedVoxelData = FNCellVoxelData();
	CachedBoundsVertices.Empty();
	RenderMode = ENWorldAssemblyEdModeRenderMode::All;

	CacheUserSettings();

	bCanTick = true;

	// Create our temp organ generator to use with any selections
	OrganGenerator = NewObject<UNAssemblyOperation>(GetTransientPackage(), NEXUS::WorldAssembly::Operations::EditorMode);
	OrganGenerator->DisplayName = FText::FromName(NEXUS::WorldAssembly::Operations::EditorMode);
	OrganGenerator->AddToRoot();

	OnLevelActorDeletedHandle = GEngine->OnLevelActorDeleted().AddStatic(&FNWorldAssemblyEdMode::OnActorDeleted);

	FEdMode::Enter();
}

void FNWorldAssemblyEdMode::Exit()
{
	CellActor = nullptr;
	bCanTick = false;

	GEngine->OnLevelActorDeleted().Remove(OnLevelActorDeletedHandle);

	// Destroy any visualizer kicking around
	DestroyCollisionVisualizer();

	// Remove our temp organ generator
	if (OrganGenerator != nullptr)
	{
		OrganGenerator->TearDownOperation();
		OrganGenerator = nullptr;
	}

	FEdMode::Exit();
}

void FNWorldAssemblyEdMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	if (bCanTick == false) return;

	// Coalesce any world changes flagged since the last tick into a single in-place rebuild of the visualizer.
	if (bCollisionVisualizerDirty && CollisionVisualizer != nullptr)
	{
		if (UWorld* VisualizerWorld = CollisionVisualizer->GetWorld())
		{
			CreateCollisionVisualizer(VisualizerWorld);
		}
		bCollisionVisualizerDirty = false;
	}

	// Resolve the cell actor for the active world. Reuse the cached pointer while it's still alive and belongs to
	// that world; only fall back to the full GetCellActorFromWorld level/actor scan when it's gone. Deletion of the
	// cached actor is handled by OnActorDeleted (it clears the pointer), a world switch is caught by the world
	// compare, and while no cell actor exists the scan repeats each tick — which is also what lets us pick one up
	// once it's added.
	const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld == nullptr)
	{
		CellActor.Reset();
	}
	else
	{
		if (const ANCellActor* Cached = CellActor.Get(); Cached == nullptr || Cached->GetWorld() != CurrentWorld)
		{
			CellActor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
		}

		if (ANCellActor* Actor = CellActor.Get(); Actor != nullptr && !Actor->WasSpawnedFromProxy())
		{
			UNCellRootComponent* RootComponent = Actor->GetCellRoot();
			const FRotator Rotation = RootComponent->GetOffsetRotator();
			const FVector Offset = RootComponent->GetOffsetLocation();

			// The hull, bounds and voxel caches are each consumed only by NCellRootComponentVisualizer, and only in
			// their matching ed-mode (hull points/edges in Hull, min/max in Bounds, the grid overlay in Voxel). Only
			// the active mode's cache is ever read, so refresh just that one rather than rebuilding all of them every
			// tick. Hull and bounds derive from the live offset transform, so they recompute each tick while active to
			// track viewport drags.
			switch (CellEdMode)
			{
			case ENCellEdMode::Hull:
				CachedHullVertices = FNVectorUtils::RotateAndOffsetPoints(RootComponent->Details.Hull.Vertices, Rotation, Offset);
				CachedHullEdges = RootComponent->Details.Hull.GetEdgeIndices();
				break;
			case ENCellEdMode::Bounds:
				CachedBounds = FNWorldAssemblyUtils::CreateRotatedBox(RootComponent->Details.Bounds, Rotation, Offset);
				CachedBoundsVertices = FNBoxUtils::GetVertices(CachedBounds);
				break;
			case ENCellEdMode::Voxel:
				// The IsEqual guard skips the array copy when the source grid is unchanged (covering in-place voxel
				// edits that keep the count) and the Origin guard catches a re-anchored grid whose contents match.
				if (const FNCellVoxelData& SourceVoxelData = RootComponent->Details.VoxelData;
					!CachedVoxelData.IsEqual(SourceVoxelData) || CachedVoxelData.GetOrigin() != SourceVoxelData.GetOrigin())
				{
					CachedVoxelData = SourceVoxelData;
				}
				break;
			}

			bAutoBoundsDisabled = !RootComponent->Details.BoundsSettings.bCalculateOnSave;
			bAutoHullDisabled = !RootComponent->Details.HullSettings.bCalculateOnSave;
			bAllowNonConvexHull = RootComponent->Details.HullSettings.bAllowNonConvex;
			bAutoVoxelDisabled = (!RootComponent->Details.VoxelSettings.bCalculateOnSave && RootComponent->Details.VoxelSettings.bUseVoxelData);
		}
	}

	FEdMode::Tick(ViewportClient, DeltaTime);
}

void FNWorldAssemblyEdMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	bHasDirtyActors = false;

	// We don't have anything to do in play mode - maybe in the future.
	if (FNEditorUtils::IsPlayInEditor() ||
		RenderMode == ENWorldAssemblyEdModeRenderMode::None ||
		RenderMode == ENWorldAssemblyEdModeRenderMode::LevelScreenshot)
	{
		FEdMode::Render(View, Viewport, PDI);
		return;
	}

	// Iterate all roots and draw their bounds
	if (FNWorldAssemblyRegistry::HasRootComponents())
	{
		for (const auto RootComponent : FNWorldAssemblyRegistry::GetCellRootComponents())
		{
			if (RootComponent == nullptr) continue;

			const ANCellActor* Actor = RootComponent->GetNCellActor();
			if (Actor != nullptr)
			{
				// Do not draw cell actors when in editor mode when spawned from proxy
				if (Actor->WasSpawnedFromProxy())
				{
					continue;
				}

				// Notice ON Dirty
				if (Actor->IsActorDirty())
				{
					bHasDirtyActors = true;
				}
			}

			// Draw debug information
			RootComponent->DrawDebugPDI(PDI, static_cast<uint8>(GetCellVoxelMode()), GetCachedCellBoundsColor(), GetCachedCellHullColor());
			// We can't use caching because we are drawing ALL of the possible roots
		}
	}
	if (FNWorldAssemblyRegistry::HasJunctionComponents() || FNWorldAssemblyRegistry::HasBoneComponents())
	{
		const UNWorldAssemblySettings* WorldAssemblySettings = UNWorldAssemblySettings::Get();
		const UNWorldAssemblyEditorUserSettings* WorldAssemblyEditorUserSettings = UNWorldAssemblyEditorUserSettings::Get();

		// Draw Junctions
		for (const auto JunctionComponent : FNWorldAssemblyRegistry::GetCellJunctionComponents())
		{
			// Bad ref?
			if (JunctionComponent == nullptr)
			{
				continue;
			}

			FNCellLinkDetails& LinkDetails = JunctionComponent->LinkDetails;
			// Author-time
			if (LinkDetails.JunctionInstanceIdentifier == -1)
			{
				JunctionComponent->DrawDebugPDI(PDI,
					WorldAssemblyEditorUserSettings->ColorPaletteJunctionsValid,
					WorldAssemblyEditorUserSettings->ColorPaletteJunctionsInvalid,
					true,
					true,
					true,
					true,
					WorldAssemblySettings);
				continue;
			}

			// Runtime Connected
			if (LinkDetails.bConnected)
			{
				const bool bConnectedDrawer = LinkDetails.ConnectedNodeIdentifier > LinkDetails.NodeIdentifier;
				JunctionComponent->DrawDebugPDI(PDI,
					WorldAssemblyEditorUserSettings->ColorPaletteJunctionsValid,
					WorldAssemblyEditorUserSettings->ColorPaletteJunctionsInvalid,
					false,true,bConnectedDrawer, bConnectedDrawer,
					WorldAssemblySettings);
				continue;
			}

			// Runtime Disconnected
			if (WorldAssemblyEditorUserSettings->bDebugWorldDrawUnfilledJunctions)
			{
				JunctionComponent->DrawDebugPDI(PDI,
					WorldAssemblyEditorUserSettings->ColorPaletteJunctionsUnfilled,
					WorldAssemblyEditorUserSettings->ColorPaletteJunctionsUnfilled,
					false,false,true, false,
					WorldAssemblySettings);
			}
		}

		// We draw the bones in world mode without socket, so that they show up in edit mode of generated stuff
		for (const auto BoneComponent : FNWorldAssemblyRegistry::GetBoneComponents())
		{
			BoneComponent->DrawDebugPDI(PDI, WorldAssemblyEditorUserSettings->ColorPaletteBonesValid, WorldAssemblyEditorUserSettings->ColorPaletteBonesInvalid, false, false);
		}
	}

	// Selection-specific drawing options
	const ENWorldAssemblySelectionFlags Flags = FNWorldAssemblyEditorUtils::GetSelectionFlags();
	if (N_FLAGS_UINT8_HAS_UINT8(Flags, ENWorldAssemblySelectionFlags::OrganVolume))
	{
		TArray<ANOrganVolume*> SelectedOrganVolumes = FNWorldAssemblyEditorUtils::GetSelectedOrganVolumes();

		// Ensure we only process organ selection when it has changed.
		if (const uint32 NewSelectedOrganHash = FNArrayUtils::GetPointersHash(SelectedOrganVolumes);
			NewSelectedOrganHash != PreviousSelectedOrganHash)
		{
			OrganGenerator->Reset();
			for (const ANOrganVolume* OrganVolume : SelectedOrganVolumes)
			{
				OrganGenerator->AddToContext(OrganVolume->GetOrganComponent());
			}
			OrganGenerator->LockContext(FNEditorUtils::GetCurrentWorld()); // We need the context locked to figure out the actual ordering

			PreviousSelectedOrganHash = NewSelectedOrganHash;
		}

		if (OrganGenerator->IsLocked())
		{
			TArray<TArray<TObjectPtr<UNOrganComponent>>>& Order = OrganGenerator->GetGenerationOrder();
			// #SONARQUBE-DISABLE-CPP_S134 Need the extra depth to iterate
			for (int32 i = 0; i < Order.Num(); i++)
			{
				for (int32 p = 0; p < Order[i].Num(); p++)
				{
					Order[i][p]->DrawDebugPDI(PDI);

					FString Label = FString::Printf(TEXT(" %i:%i %s"), i, p, *Order[i][p]->GetDebugLabel());

					FNPositionRotation LabelOrientation = Order[i][p]->GetDebugLabelPositionRotation();
					FNPrimitiveFont::DrawPDI(PDI, Label,
					 	LabelOrientation.Position, LabelOrientation.Rotation, FLinearColor::White);
				}
			}
			// #SONARQUBE-ENABLE
		}
	}
	else if ( OrganGenerator->IsLocked())
	{
		OrganGenerator->Reset();
		PreviousSelectedOrganHash = 0;
	}

	FEdMode::Render(View, Viewport, PDI);
}

void FNWorldAssemblyEdMode::DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	// Messages disabled
	if (!UNWorldAssemblyEditorUserSettings::Get()->bNotificationsDisplayViewportMessages || FNEditorUtils::IsPlayInEditor() ||
		RenderMode != ENWorldAssemblyEdModeRenderMode::All)
	{
		FEdMode::DrawHUD(ViewportClient, Viewport, View, Canvas);
		return;
	}

	CanvasMessageBox.Clear();

	if (bHasDirtyActors)
	{
		CanvasMessageBox.AddSeverity(ENSeverity::Warning);
		CanvasMessageBox.AddSmallLine(DirtyMessage, FLinearColor::Yellow);
	}

	if (bAutoBoundsDisabled && bAutoHullDisabled)
	{
		CanvasMessageBox.AddSmallLine(AutoBoundsHullMessage);
	}
	else if (bAutoBoundsDisabled)
	{
		CanvasMessageBox.AddSmallLine(AutoBoundsMessage);
	}
	else if (bAutoHullDisabled)
	{
		CanvasMessageBox.AddSmallLine(AutoHullMessage);
	}
	else if (bAutoVoxelDisabled)
	{
		CanvasMessageBox.AddSmallLine(AutoVoxelMessage);
	}

	if (CanvasMessageBox.HasContent())
	{
		FNCanvasUtils::DrawCanvasTextBox(&CanvasMessageBox, Canvas, FVector2D(10,10));
	}

	FEdMode::DrawHUD(ViewportClient, Viewport, View, Canvas);
}

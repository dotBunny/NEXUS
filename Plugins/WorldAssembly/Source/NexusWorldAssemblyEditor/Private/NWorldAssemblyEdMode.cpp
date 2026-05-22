// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEdMode.h"

#include "NCanvasUtils.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"
#include "NWorldAssemblyRegistry.h"
#include "Cell/NCellRootComponent.h"
#include "NEditorUtils.h"
#include "Developer/NPrimitiveFont.h"
#include "NWorldAssemblyEditorUtils.h"
#include "NWorldAssemblyUtils.h"
#include "Macros/NFlagsMacros.h"
#include "Math/NBoxUtils.h"
#include "Math/NVectorUtils.h"
#include "Assembly/NAssemblyOperation.h"
#include "Developer/NMethodScopeTimer.h"

void FNWorldAssemblyEdMode::ProtectCellEdMode()
{
	if (CellActor != nullptr && CellActor->GetCellRoot()->Details.Hull.HasNonTris() && 
		GetCellEdMode() == ENCellEdMode::Hull)
	{
		SetCellEdMode(ENCellEdMode::Bounds);
	}
}

void FNWorldAssemblyEdMode::OnActorDeleted(AActor* Actor)
{
	if (Actor == CollisionVisualizer)
	{
		CollisionVisualizer = nullptr;
	}
}

TObjectPtr<ANDebugActor> FNWorldAssemblyEdMode::CreateCollisionVisualizer(UWorld* World)
{
	DestroyCollisionVisualizer();
	FNMethodScopeTimer("World Collision Build Time");
	CollisionVisualizer = FNWorldAssemblyEditorUtils::CreateWorldCollisionVisualizerActor(World, TArray<FBoxSphereBounds>());
	return CollisionVisualizer;
}

void FNWorldAssemblyEdMode::DestroyCollisionVisualizer()
{
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

const FEditorModeID FNWorldAssemblyEdMode::Identifier = TEXT("NWorldAssemblyEdMode");
const FText FNWorldAssemblyEdMode::DirtyMessage = FText::FromString("Dirty Cell Actor");
const FText FNWorldAssemblyEdMode::AutoBoundsMessage = FText::FromString("Cell Bounds not calculated on save.");
const FText FNWorldAssemblyEdMode::AutoBoundsHullMessage = FText::FromString("Cell Bounds and Hull not calculated on save.");
const FText FNWorldAssemblyEdMode::AutoHullMessage = FText::FromString("Cell Hull not calculated on save.");
const FText FNWorldAssemblyEdMode::AutoVoxelMessage = FText::FromString("Cell Voxel not calculated on save.");

ANCellActor* FNWorldAssemblyEdMode::CellActor = nullptr;
FNWorldAssemblyEdMode::ENCellEdMode FNWorldAssemblyEdMode::CellEdMode = ENCellEdMode::Bounds;
TArray<FVector> FNWorldAssemblyEdMode::CachedHullVertices;
TArray<FIntVector2> FNWorldAssemblyEdMode::CachedHullEdges;
FLinearColor FNWorldAssemblyEdMode::CachedHullColor = FColor::Blue;
FBox FNWorldAssemblyEdMode::CachedBounds;
FNCellVoxelData FNWorldAssemblyEdMode::CachedVoxelData;
FLinearColor FNWorldAssemblyEdMode::CachedBoundsColor = FColor::Red;
TArray<FVector> FNWorldAssemblyEdMode::CachedBoundsVertices;
FNWorldAssemblyEdMode::ENCellVoxelMode FNWorldAssemblyEdMode::CellVoxelMode = ENCellVoxelMode::None;
TObjectPtr<ANDebugActor> FNWorldAssemblyEdMode::CollisionVisualizer = nullptr;

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
	CachedHullColor = FColor::Blue;
	CachedBounds = FBox(ForceInit);
	CachedVoxelData = FNCellVoxelData();
	CachedBoundsColor = FColor::Red;
	CachedBoundsVertices.Empty();

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
	
	// Cache if we have a NCellActor setup
	CellActor = nullptr;
	
	if (const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld(); CurrentWorld != nullptr)
	{
		CellActor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
		if (CellActor != nullptr && !CellActor->WasSpawnedFromProxy())
		{
			UNCellRootComponent* RootComponent = CellActor->GetCellRoot();
			const FRotator Rotation = RootComponent->GetOffsetRotator();;
			const FVector Offset = RootComponent->GetOffsetLocation();

			// We only update them during tick
			CachedHullVertices = FNVectorUtils::RotateAndOffsetPoints(RootComponent->Details.Hull.Vertices, Rotation, Offset);
			CachedHullEdges = RootComponent->Details.Hull.GetEdgeIndices();
			CachedBounds = FNWorldAssemblyUtils::CreateRotatedBox(RootComponent->Details.Bounds, Rotation, Offset);
			CachedBoundsVertices = FNBoxUtils::GetVertices(CachedBounds);
			CachedVoxelData = RootComponent->Details.VoxelData;

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
			RootComponent->DrawDebugPDI(PDI, static_cast<uint8>(GetCellVoxelMode())); // We can't use caching because we are drawing ALL of the possible roots
		}
	}
	if (FNWorldAssemblyRegistry::HasJunctionComponents())
	{
		for (const auto JunctionComponent : FNWorldAssemblyRegistry::GetCellJunctionComponents())
		{
			if (JunctionComponent == nullptr) continue;
			JunctionComponent->DrawDebugPDI(PDI);
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

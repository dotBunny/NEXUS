// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEdMode.h"

#include "CanvasTypes.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"
#include "NProcGenRegistry.h"
#include "Cell/NCellRootComponent.h"
#include "NEditorUtils.h"
#include "Developer/NPrimitiveFont.h"
#include "NProcGenEditorUtils.h"
#include "NProcGenUtils.h"
#include "Macros/NFlagsMacros.h"
#include "Math/NBoxUtils.h"
#include "Math/NVectorUtils.h"
#include "NProcGenOperation.h"

void FNProcGenEdMode::ProtectCellEdMode()
{
	if (CellActor != nullptr && CellActor->GetCellRoot()->Details.Hull.HasNonTris() && 
		GetCellEdMode() == ENCellEdMode::Hull)
	{
		SetCellEdMode(ENCellEdMode::Bounds);
	}
}

const FEditorModeID FNProcGenEdMode::Identifier = TEXT("NProcGenEdMode");
const FText FNProcGenEdMode::DirtyMessage = FText::FromString("Dirty NCellActor");
const FText FNProcGenEdMode::AutoBoundsMessage = FText::FromString("NCell Bounds not calculated on save.");
const FText FNProcGenEdMode::AutoBoundsHullMessage = FText::FromString("NCell Bounds and Hull not calculated on save.");
const FText FNProcGenEdMode::AutoHullMessage = FText::FromString("NCell Hull not calculated on save.");
const FText FNProcGenEdMode::AutoVoxelMessage = FText::FromString("NCell Voxel not calculated on save.");

ANCellActor* FNProcGenEdMode::CellActor = nullptr;
FNProcGenEdMode::ENCellEdMode FNProcGenEdMode::CellEdMode = ENCellEdMode::Bounds;
TArray<FVector> FNProcGenEdMode::CachedHullVertices;
FLinearColor FNProcGenEdMode::CachedHullColor = FColor::Blue;
FBox FNProcGenEdMode::CachedBounds;
FNCellVoxelData FNProcGenEdMode::CachedVoxelData;
FLinearColor FNProcGenEdMode::CachedBoundsColor = FColor::Red;
TArray<FVector> FNProcGenEdMode::CachedBoundsVertices;
FNProcGenEdMode::ENCellVoxelMode FNProcGenEdMode::CellVoxelMode = ENCellVoxelMode::None;

FNProcGenEdMode::~FNProcGenEdMode()
{
	if (OrganGenerator != nullptr)
	{
		OrganGenerator->Reset();
		OrganGenerator->RemoveFromRoot();
		OrganGenerator->ConditionalBeginDestroy();
		OrganGenerator = nullptr;
	}
}

void FNProcGenEdMode::Enter()
{
	// Initialize cached values
	CellActor = nullptr;
	CachedHullVertices.Empty();
	CachedHullColor = FColor::Blue;
	CachedBounds = FBox(ForceInit);
	CachedVoxelData = FNCellVoxelData();
	CachedBoundsColor = FColor::Red;
	CachedBoundsVertices.Empty();

	bCanTick = true;

	// Create our temp organ generator to use with any selections
	OrganGenerator = NewObject<UNProcGenOperation>(GetTransientPackage(), NEXUS::ProcGen::Operations::EditorMode);
	OrganGenerator->DisplayName = FText::FromName(NEXUS::ProcGen::Operations::EditorMode);
	OrganGenerator->AddToRoot();
	
	FEdMode::Enter();
}

void FNProcGenEdMode::Exit()
{
	CellActor = nullptr;
	bCanTick = false;

	// Remove our temp organ generator
	if (OrganGenerator != nullptr)
	{
		OrganGenerator->Reset();
		OrganGenerator->RemoveFromRoot();
		OrganGenerator->ConditionalBeginDestroy();
		OrganGenerator = nullptr;
	}
	
	FEdMode::Exit();
}

void FNProcGenEdMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	if (bCanTick == false) return;
	
	// Cache if we have a NCellActor setup
	CellActor = nullptr;
	
	if (const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld(); CurrentWorld != nullptr)
	{
		CellActor = FNProcGenUtils::GetCellActorFromWorld(CurrentWorld, true);
		if (CellActor != nullptr)
		{
			UNCellRootComponent* RootComponent = CellActor->GetCellRoot();
			const FRotator Rotation = RootComponent->GetOffsetRotator();;
			const FVector Offset = RootComponent->GetOffsetLocation();

			// We only update them during tick
			CachedHullVertices = FNVectorUtils::RotateAndOffsetPoints(RootComponent->Details.Hull.Vertices, Rotation, Offset);
			CachedBounds = FNProcGenUtils::CreateRotatedBox(RootComponent->Details.Bounds, Rotation, Offset);
			CachedBoundsVertices = FNBoxUtils::GetVertices(CachedBounds);
			CachedVoxelData = RootComponent->Details.VoxelData;

			bAutoBoundsDisabled = !RootComponent->Details.BoundsSettings.bCalculateOnSave;
			bAutoHullDisabled = !RootComponent->Details.HullSettings.bCalculateOnSave;
			bAutoVoxelDisabled = (!RootComponent->Details.VoxelSettings.bCalculateOnSave && RootComponent->Details.VoxelSettings.bUseVoxelData);
		}
	}
	
	FEdMode::Tick(ViewportClient, DeltaTime);
}

void FNProcGenEdMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	bHasDirtyActors = false;
	
	// Iterate all roots and draw their bounds
	if (FNProcGenRegistry::HasRootComponents())
	{
		for (const auto RootComponent : FNProcGenRegistry::GetCellRootComponents())
		{
			if (RootComponent == nullptr) continue;
			RootComponent->DrawDebugPDI(PDI, static_cast<uint8>(GetCellVoxelMode())); // We can't use caching because we are drawing ALL of the possible roots

			// Notice ON Dirty
			const ANCellActor* Actor = Cast<ANCellActor>(RootComponent->GetOwner());
			if (Actor != nullptr && Actor->IsActorDirty())
			{
				bHasDirtyActors = true;
			}
		}
	}
	if (FNProcGenRegistry::HasJunctionComponents())
	{
		for (const auto JunctionComponent : FNProcGenRegistry::GetCellJunctionComponents())
		{
			if (JunctionComponent == nullptr) continue;
			JunctionComponent->DrawDebugPDI(PDI);
		}	
	}

	// Selection-specific drawing options
	const ENProcGenSelectionFlags Flags = FNProcGenEditorUtils::GetSelectionFlags();
	if (N_FLAGS_UINT8_HAS_UINT8(Flags, ENProcGenSelectionFlags::OrganVolume))
	{
		TArray<ANOrganVolume*> SelectedOrganVolumes = FNProcGenEditorUtils::GetSelectedOrganVolumes();

		// Ensure we only process organ selection when it has changed.
		if (const uint32 NewSelectedOrganHash = FNArrayUtils::GetPointersHash(SelectedOrganVolumes); 
			NewSelectedOrganHash != PreviousSelectedOrganHash)
		{
			OrganGenerator->Reset();
			for (const ANOrganVolume* OrganVolume : SelectedOrganVolumes)
			{
				OrganGenerator->AddToContext(OrganVolume->GetOrganComponent());
			}
			OrganGenerator->LockContext(); // We need the context locked to figure out the actual ordering
			
			PreviousSelectedOrganHash = NewSelectedOrganHash;
		}
		
		if (OrganGenerator->IsLocked())
		{
			TArray<TArray<UNOrganComponent*>>& Order = OrganGenerator->GetGenerationOrder();
			// #SONARQUBE-DISABLE-CPP_S134 Need the extra depth to iterate
			for (int i = 0; i < Order.Num(); i++)
			{
				for (int p = 0; p < Order[i].Num(); p++)
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

void FNProcGenEdMode::DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	int MessageOffset = 35;
	
	if (bHasDirtyActors)
	{
		Canvas->DrawShadowedText(10,MessageOffset, DirtyMessage, GEngine->GetSmallFont(), FLinearColor::Yellow);
		MessageOffset += MessageSpacing;
	}

	if (bAutoBoundsDisabled && bAutoHullDisabled)
	{
		Canvas->DrawShadowedText(10,MessageOffset, AutoBoundsHullMessage, GEngine->GetSmallFont(), FLinearColor::White);
		MessageOffset += MessageSpacing;	
	}
	else if (bAutoBoundsDisabled)
	{
		Canvas->DrawShadowedText(10,MessageOffset, AutoBoundsMessage, GEngine->GetSmallFont(), FLinearColor::White);
		MessageOffset += MessageSpacing;	
	}
	else if (bAutoHullDisabled)
	{

		Canvas->DrawShadowedText(10,MessageOffset, AutoHullMessage, GEngine->GetSmallFont(), FLinearColor::White);
		MessageOffset += MessageSpacing;	
	}
	else if (bAutoVoxelDisabled)
	{

		Canvas->DrawShadowedText(10,MessageOffset, AutoVoxelMessage, GEngine->GetSmallFont(), FLinearColor::White);
		MessageOffset += MessageSpacing;	
	}
	
	FEdMode::DrawHUD(ViewportClient, Viewport, View, Canvas);
}

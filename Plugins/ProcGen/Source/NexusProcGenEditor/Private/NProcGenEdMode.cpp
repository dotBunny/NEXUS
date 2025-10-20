// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEdMode.h"

#include "CanvasTypes.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"
#include "NProcGenRegistry.h"
#include "Cell/NCellRootComponent.h"
#include "NEditorUtils.h"
#include "NProcGenEditorUtils.h"
#include "NProcGenUtils.h"
#include "NPrimitiveDrawingUtils.h"
#include "Macros/NFlagsMacros.h"
#include "Math/NBoxUtils.h"
#include "Math/NVectorUtils.h"
#include "Organ/NOrganGenerator.h"

const FEditorModeID FNProcGenEdMode::Identifier = TEXT("NProcGenEdMode");
const FText FNProcGenEdMode::DirtyMessage = FText::FromString("Dirty NCellActor");
const FText FNProcGenEdMode::AutoBoundsMessage = FText::FromString("NCell Bounds not calculated on save.");
const FText FNProcGenEdMode::AutoBoundsHullMessage = FText::FromString("NCell Bounds and Hull not calculated on save.");
const FText FNProcGenEdMode::AutoHullMessage = FText::FromString("NCell Hull not calculated on save.");

ANCellActor* FNProcGenEdMode::NCellActor = nullptr;
FNProcGenEdMode::ENCellEdMode FNProcGenEdMode::NCellEdMode = NCell_Bounds;
TArray<FVector> FNProcGenEdMode::CachedHullVertices;
FLinearColor FNProcGenEdMode::CachedHullColor = FColor::Blue;
FBox FNProcGenEdMode::CachedBounds;
FLinearColor FNProcGenEdMode::CachedBoundsColor = FColor::Red;
TArray<FVector> FNProcGenEdMode::CachedBoundsVertices;

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
	NCellActor = nullptr;
	CachedHullVertices.Empty();
	CachedHullColor = FColor::Blue;
	CachedBounds = FBox(ForceInit);
	CachedBoundsColor = FColor::Red;
	CachedBoundsVertices.Empty();

	bCanTick = true;

	// Create our temp organ generator to use with any selections
	OrganGenerator = NewObject<UNOrganGenerator>();
	OrganGenerator->AddToRoot();
	
	FEdMode::Enter();
}

void FNProcGenEdMode::Exit()
{
	NCellActor = nullptr;
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
	NCellActor = nullptr;
	
	if (const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld(); CurrentWorld != nullptr)
	{
		NCellActor = FNProcGenUtils::GetCellActorFromWorld(CurrentWorld, true);
		if (NCellActor != nullptr)
		{
			UNCellRootComponent* RootComponent = NCellActor->GetCellRoot();
			const FRotator Rotation = RootComponent->GetOffsetRotator();;
			const FVector Offset = RootComponent->GetOffsetLocation();

			// We only update them during tick
			CachedHullVertices = FNVectorUtils::RotateAndOffsetVectors(RootComponent->Details.Hull.Vertices, Rotation, Offset);
			CachedBounds = FNProcGenUtils::CreateRotatedBox(RootComponent->Details.Bounds, Rotation, Offset);
			CachedBoundsVertices = FNBoxUtils::GetVertices(CachedBounds);

			bAutoBoundsDisabled = !RootComponent->Details.BoundsSettings.bCalculateOnSave;
			bAutoHullDisabled = !RootComponent->Details.HullSettings.bCalculateOnSave;
		}
	}
	
	FEdMode::Tick(ViewportClient, DeltaTime);
}

void FNProcGenEdMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	bHasDirtyActors = false;
	
	FString Test = TEXT("ABCDEFGHIJKL MNOPQRSTUVWXYZ\nABC\n1234567890\n-[]():\t@+#\nabc def ghi jkl mno pqr stu vwx yz\nA'BCDE,FGH.IJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ");
	FNPrimitiveDrawingUtils::DrawString(PDI, Test, FVector(0.f, 0.f, 1000.f), FRotator::ZeroRotator);
	
	// Iterate all roots and draw their bounds
	if (FNProcGenRegistry::HasRootComponents())
	{
		for (const auto RootComponent : FNProcGenRegistry::GetCellRootComponents())
		{
			if (RootComponent == nullptr) continue;
			RootComponent->DrawDebugPDI(PDI); // We can't use caching because we are drawing ALL of the possible roots

			// Notice ON Dirty
			if (const ANCellActor* CellActor = Cast<ANCellActor>(RootComponent->GetOwner()))
			{
				if (CellActor->IsActorDirty())
				{
					bHasDirtyActors = true;
				}
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
	if (N_FLAGS_HAS(Flags, PGSF_OrganVolume))
	{
		TArray<ANOrganVolume*> SelectedOrganVolumes = FNProcGenEditorUtils::GetSelectedOrganVolumes();

		// Ensure we only process organ selection when it has changed.
		if (const uint32 NewSelectedOrganHash = FNArrayUtils::GetPointersHash(SelectedOrganVolumes); 
			NewSelectedOrganHash != PreviousSelectedOrganHash)
		{
			for (const ANOrganVolume* OrganVolume : SelectedOrganVolumes)
			{
				OrganGenerator->Reset();
				OrganGenerator->AddToContext(OrganVolume->OrganComponent);
				OrganGenerator->LockContext(); // We need the context locked to figure out the actual ordering
			}
			PreviousSelectedOrganHash = NewSelectedOrganHash;
		}
		
		if (OrganGenerator->IsLocked())
		{
			TArray<TArray<UNOrganComponent*>>& Order = OrganGenerator->GetGenerationOrder();
			for (int i = 0; i < Order.Num(); i++)
			{
				for (int p = 0; p < Order[i].Num(); p++)
				{
					// TODO: this should be gradient?
					FString Label = FString::Printf(TEXT("%i:%i %s"), i, p, *Order[i][p]->GetDebugLabel());
					FNPositionRotation LabelOrientation = Order[i][p]->GetDebugLabelPositionRotation();
					FNPrimitiveDrawingUtils::DrawString(PDI, Label,
					 	LabelOrientation.Position, LabelOrientation.Rotation, FLinearColor::White);
					
					Order[i][p]->DrawDebugPDI(PDI, FLinearColor::White);
				}
			}
		}
	}
	else if ( OrganGenerator->IsLocked())
	{
		OrganGenerator->Reset();
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
	
	FEdMode::DrawHUD(ViewportClient, Viewport, View, Canvas);
}

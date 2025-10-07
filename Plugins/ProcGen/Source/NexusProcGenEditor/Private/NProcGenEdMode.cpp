// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEdMode.h"

#include "CanvasTypes.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"
#include "Cell/NCellRegistry.h"
#include "Cell/NCellRootComponent.h"
#include "NEditorUtils.h"
#include "NProcGenUtils.h"
#include "Math/NBoxUtils.h"
#include "Math/NVectorUtils.h"

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
	
	FEdMode::Enter();
}

void FNProcGenEdMode::Exit()
{
	NCellActor = nullptr;
	bCanTick = false;
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
	
	// Iterate all roots and draw their bounds
	if (FNCellRegistry::HasRootComponents())
	{
		
		for (const auto RootComponent : FNCellRegistry::GetRootComponents())
		{
			if (RootComponent == nullptr) continue;
			RootComponent->DrawDebugPDI(PDI); // We cant use caching because we are drawing ALL fo the possible roots

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
	if (FNCellRegistry::HasJunctionComponents())
	{
		for (const auto JunctionComponent : FNCellRegistry::GetJunctionComponents())
		{
			if (JunctionComponent == nullptr) continue;
			JunctionComponent->DrawDebugPDI(PDI);
		}	
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

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Visualizers/NBoneComponentVisualizer.h"

#include "NProcGenEdMode.h"
#include "Organ/NBoneComponent.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

void FNBoneComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View,
	FPrimitiveDrawInterface* PDI)
{
	// First we check that we indeed have an actor component
	const UNBoneComponent* BoneComponent = Cast<UNBoneComponent>(const_cast<UActorComponent*>(Component));
	if (!BoneComponent)
	{
		return;
	}

	// We need to draw the base wireframes
	if (!FNProcGenEdMode::IsActive())
	{
		BoneComponent->DrawDebugPDI(PDI);
	}
}

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Visualizers/NCellJunctionComponentVisualizer.h"
#include "Cell/NCellJunctionComponent.h"
#include "NProcGenEdMode.h"

void FNCellJunctionComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	// First we check that we indeed have an actor component
	const UNCellJunctionComponent* JunctionComponent = Cast<UNCellJunctionComponent>(const_cast<UActorComponent*>(Component));
	if (!JunctionComponent)
	{
		return;
	}

	// We need to draw the base wireframes
	if (!FNProcGenEdMode::IsActive())
	{
		JunctionComponent->DrawDebugPDI(PDI);
	}
}

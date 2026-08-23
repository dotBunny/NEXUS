// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Visualizers/NCellJunctionComponentVisualizer.h"
#include "Cell/NCellJunctionComponent.h"
#include "NWorldAssemblyEditorColors.h"
#include "NWorldAssemblyEditorUserSettings.h"
#include "EdMode/NWorldAssemblyEdMode.h"

void FNCellJunctionComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	// First we check that we indeed have an actor component
	const UNCellJunctionComponent* JunctionComponent = Cast<UNCellJunctionComponent>(const_cast<UActorComponent*>(Component));
	if (!JunctionComponent)
	{
		return;
	}

	// We need to draw the base wireframes
	if (!UNWorldAssemblyEdMode::IsActive())
	{
		// Every argument from here on is the call's own default but the fill-depth flag, which has to be stated
		// positionally to reach it.
		JunctionComponent->DrawDebugPDI(PDI,
			FNWorldAssemblyEditorColors::GetJunctionValid(),
			FNWorldAssemblyEditorColors::GetJunctionInvalid(), false, true, true, true,
			UNWorldAssemblyEditorUserSettings::Get()->bDebugCellDrawFillBounds);
	}
}

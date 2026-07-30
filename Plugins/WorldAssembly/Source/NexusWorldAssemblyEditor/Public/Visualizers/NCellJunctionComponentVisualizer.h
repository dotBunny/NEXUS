// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "ComponentVisualizer.h"

/**
 * Component visualizer for UNCellJunctionComponent — draws the junction socket footprint and
 * orientation gizmo in the level viewport so authors can see connection points without selecting.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-types/visualizers/cell-junction-component-visualizer/">FNCellJunctionComponentVisualizer</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNCellJunctionComponentVisualizer final : public FComponentVisualizer
{
public:
	//~FComponentVisualizer
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	//End FComponentVisualizer
};

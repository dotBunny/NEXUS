// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "ComponentVisualizer.h"

class UNCellRootComponent;

/**
 * Component visualizer for UNCellRootComponent, drawing the cell's bounds/hull/voxel overlays in-viewport.
 *
 * Draw-only. The hit-proxy editing this used to carry — dragging hull and bounds vertices, toggling voxel points,
 * selecting an edge to split — moved to UNCellBoundsTool, UNCellHullVertexTool and UNCellVoxelTool when the World Assembly
 * edit mode gained interactive tools. What is left is the wireframe pass for when the mode is *not* active, which the
 * tools cannot cover because they only exist while it is.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-types/visualizers/cell-root-component-visualizer/">FNCellRootComponentVisualizer</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNCellRootComponentVisualizer final : public FComponentVisualizer
{
public:
	//~FComponentVisualizer
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	//End FComponentVisualizer
};

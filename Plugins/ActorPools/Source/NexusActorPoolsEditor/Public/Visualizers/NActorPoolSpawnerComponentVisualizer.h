// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "ComponentVisualizer.h"

/**
 * Editor component visualizer that draws the distribution shape of a UNActorPoolSpawnerComponent
 * in level viewports.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/editor-types/visualizers/actor-pool-spawner-component-visualizer/">FNActorPoolSpawnerComponentVisualizer</a>
 */
class FNActorPoolSpawnerComponentVisualizer final : public FComponentVisualizer
{
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};
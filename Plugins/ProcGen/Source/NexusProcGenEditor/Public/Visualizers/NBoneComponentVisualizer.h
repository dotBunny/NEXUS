// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "ComponentVisualizer.h"


class UNBoneComponent;

/**
 * Component visualizer for UNBoneComponent — renders the bone's socket, reach, and mode-specific
 * widgets in the level viewport so authors can reason about junction anchors without selection.
 */
class NEXUSPROCGENEDITOR_API FNBoneComponentVisualizer final : public FComponentVisualizer
{
public:
	//~FComponentVisualizer
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	//End FComponentVisualizer
};
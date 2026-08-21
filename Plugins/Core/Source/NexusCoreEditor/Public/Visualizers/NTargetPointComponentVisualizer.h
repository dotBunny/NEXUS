// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "ComponentVisualizer.h"

/**
 * Component visualizer for UNTargetPointComponent — draws the marker's orientation and scale in the level
 * viewport, and labels it with the component tags Get Component Points filters on.
 *
 * This is the whole of a target point's presence in the world: the component itself attaches nothing, so what
 * is drawn here is all there is to see and all there is to click. Get Component Points emits the component
 * transform whole, so rotation and scale reach every spawner downstream and both are drawn, not just position.
 *
 * What it draws is also what is clicked: the marker carries a hit proxy, so clicking it selects the component
 * itself rather than the actor holding it, which is what puts the transform gizmo on the marker being moved.
 *
 * @note Component visualizers are only drawn for the components of a selected actor, so markers are visible
 *       while their actor is selected and not otherwise. That is deliberate — see UNTargetPointComponent.
 * @note Accepting a click makes this the active component visualizer, which would block alt-drag duplication;
 *       TrackingStarted gives that status back. Read both comments together before changing either.
 * @see <a href="https://nexus-framework.com/docs/plugins/core/editor-types/visualizers/target-point-component-visualizer/">FNTargetPointComponentVisualizer</a>
 */
class NEXUSCOREEDITOR_API FNTargetPointComponentVisualizer final : public FComponentVisualizer
{
public:
	//~FComponentVisualizer
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	virtual bool ShouldAutoSelectElementOnHandleClick() const override;
	virtual void TrackingStarted(FEditorViewportClient* InViewportClient) override;
	//End FComponentVisualizer
};

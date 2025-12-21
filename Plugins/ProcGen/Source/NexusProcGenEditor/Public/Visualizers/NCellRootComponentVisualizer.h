// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "ComponentVisualizer.h"


class UNCellRootComponent;

class NEXUSPROCGENEDITOR_API FNCellRootComponentVisualizer final : public FComponentVisualizer
{
	enum EEditMode
	{
		EM_None,
		EM_HullVertex,
		EM_BoundsVertex
	};
	
public:
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void EndEditing() override;

	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	virtual bool HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;

private:
	const float PointSize = 12.0f;
	bool EditHullVertex(UNCellRootComponent* Component, int32 Index);
	bool EditBoundsVertex(UNCellRootComponent* Component, int32 Index);
	static bool ToggleVoxelPoint(UNCellRootComponent* Component, int32 Index);
	
	EEditMode CurrentEditMode = EEditMode::EM_None;

	int32 VertexIndex = -1;
	UNCellRootComponent* RootComponent = nullptr;
	
	// DISABLE AUTO SAVE when you move something
	// Draw Root When Selected (bounds/hull)
};

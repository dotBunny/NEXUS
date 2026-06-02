// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "ComponentVisualizer.h"


class UNCellRootComponent;

/**
 * Component visualizer for UNCellRootComponent that both renders the cell's bounds/hull/voxel
 * overlays in-viewport and accepts drag/click input to edit individual hull/bounds vertices or
 * toggle voxel points — depending on which cell-edit sub-mode is active.
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNCellRootComponentVisualizer final : public FComponentVisualizer
{
public:
	/**
	 * Which vertex/point type is currently being dragged, if any.
	 */
	enum class ENCellEditMode
	{
		None = 0,
		HullVertex = 1,
		BoundsVertex = 2,
		HullEdge = 3,
	};
	
	//~FComponentVisualizer
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void EndEditing() override;

	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	virtual bool HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;

	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	//End FComponentVisualizer
	
	ENCellEditMode GetMode() const { return CurrentEditMode; }
	/** @return true if a full hull edge (both endpoints) is currently selected. */
	bool HasEdgeSelected() const { return EdgeStartIndex != -1 && EdgeEndIndex != -1; }
	/** @return The selected hull edge as (start vertex index, end vertex index). */
	FIntVector2 GetEdgeSelection() const { return FIntVector2(EdgeStartIndex, EdgeEndIndex); }
	/** Clear the current vertex and edge selection. */
	void ClearSelection()
	{
		VertexIndex = -1;
		EdgeStartIndex = -1;
		EdgeEndIndex = -1;
	}
private:
	/** Pixel size used when rendering selectable vertex points. */
	const float PointSize = 12.0f;

	bool EditHullEdge(UNCellRootComponent* Component, int32 IndexA, int32 IndexB);
	
	/** Apply a drag delta to the hull vertex at Index; returns true if the component was mutated. */
	bool EditHullVertex(UNCellRootComponent* Component, int32 Index);

	/** Apply a drag delta to the bounds vertex at Index; returns true if the component was mutated. */
	bool EditBoundsVertex(UNCellRootComponent* Component, int32 Index);

	/** Toggle a single voxel at Index on or off; returns true if the component was mutated. */
	static bool ToggleVoxelPoint(UNCellRootComponent* Component, int32 Index);

	/** Active drag mode between a click and EndEditing. */
	ENCellEditMode CurrentEditMode = ENCellEditMode::None;

	/** Index of the vertex or voxel currently being dragged (−1 when inactive). */
	int32 VertexIndex = -1;
	int32 EdgeStartIndex = -1;
	int32 EdgeEndIndex = -1;

	/** Component currently targeted by the drag; cleared by EndEditing. */
	TObjectPtr<UNCellRootComponent> RootComponent = nullptr;
};

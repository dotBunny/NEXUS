// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "Tools/NCellToolBase.h"

#include "NCellHullSplitTool.generated.h"

/** Builds UNCellHullSplitTool for the edit mode's focused cell. */
UCLASS()
class NEXUSWORLDASSEMBLYEDITOR_API UNCellHullSplitToolBuilder : public UNCellToolBuilderBase
{
	GENERATED_BODY()

public:
	//~UInteractiveToolBuilder
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
	//End UInteractiveToolBuilder
};

/**
 * Splits the focused cell's hull edges: click an edge to insert a vertex at the point clicked.
 *
 * Replaces the Split Hull Edge action, which split at the midpoint of an edge the hull tool had to select first. The
 * split point is the point on the edge closest to the click ray, so the vertex lands where the user aimed rather than
 * where the edge happens to be halved.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
UCLASS()
class NEXUSWORLDASSEMBLYEDITOR_API UNCellHullSplitTool : public UNCellToolBase, public IClickBehaviorTarget, public IHoverBehaviorTarget
{
	GENERATED_BODY()

public:
	//~UInteractiveTool
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;

	/**
	 * Draw the edge under the cursor and the point a click would split it at.
	 * @note Only those. The existing vertices are not this tool's subject — it adds one — and drawing handles on them
	 *       would advertise a pick it does not offer.
	 */
	virtual void Render(IToolsContextRenderAPI* RenderAPI) override;
	//End UInteractiveTool

	//~IClickBehaviorTarget
	virtual FInputRayHit IsHitByClick(const FInputDeviceRay& ClickPos) override;
	virtual void OnClicked(const FInputDeviceRay& ClickPos) override;
	//End IClickBehaviorTarget

	//~IHoverBehaviorTarget
	virtual FInputRayHit BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos) override;
	virtual void OnBeginHover(const FInputDeviceRay& DevicePos) override;
	virtual bool OnUpdateHover(const FInputDeviceRay& DevicePos) override;
	virtual void OnEndHover() override;
	//End IHoverBehaviorTarget

protected:
	//~UNCellToolBase
	virtual TOptional<UNWorldAssemblyEdMode::ENCellEdMode> GetCellEdMode() const override { return UNWorldAssemblyEdMode::ENCellEdMode::Hull; }
	//End UNCellToolBase

private:
	/** An edge picked out from under a ray, and where along it the split would land. */
	struct FNEdgeHit
	{
		/** Index into the mode's cached hull edges, or INDEX_NONE when nothing was picked. */
		int32 EdgeIndex = INDEX_NONE;

		/** Distance along the ray of the closest approach, doubling as the pick depth. */
		double RayParameter = 0.0;

		/** World-space point on the edge nearest the ray — where the new vertex goes. */
		FVector SplitPosition = FVector::ZeroVector;

		bool IsValid() const { return EdgeIndex != INDEX_NONE; }
	};

	/**
	 * @param Ray Click ray in world space.
	 * @return The hull edge nearest the ray within the pick threshold, and the point on it to split at.
	 * @note Only considers edges on a face turned toward the ray's origin. Without that the test is pure ray-to-point
	 *       distance, and the far side of the hull competes on equal footing with the near side — so aiming at open
	 *       space inside the silhouette picks whatever edge happens to project there, which is usually behind.
	 * @remark Back-face culling, not true occlusion: it does not account for level geometry in front of the hull, and
	 *         on a non-convex hull a facing edge can still be hidden behind another part of the same hull.
	 */
	FNEdgeHit FindEdgeUnderRay(const FRay& Ray) const;

	/**
	 * @param Ray Ray to measure against.
	 * @param Start First endpoint of the segment.
	 * @param End Second endpoint of the segment.
	 * @return The point on the segment closest to Ray, held clear of both endpoints.
	 * @note Solved rather than sampled, unlike the pick above it: a pick only has to answer which edge, but the split
	 *       position is what the user sees land, and snapping it to one of eight sample points would be visible.
	 */
	static FVector FindClosestPointOnSegment(const FRay& Ray, const FVector& Start, const FVector& End);

	/** Insert a vertex into the hovered edge at the hovered position. */
	void SplitHoveredEdge();

	/** The edge under the cursor, refreshed on every hover update and cleared when the cursor leaves. */
	FNEdgeHit HoveredEdge;

	/** Pixel size of the marker previewing where a click would split. */
	static constexpr float SplitPointSize = 16.0f;

	/** Line thickness of the highlight drawn over the hovered edge. */
	static constexpr float HoveredEdgeThickness = 4.0f;

	/**
	 * How far along an edge a split is held clear of either endpoint, as a fraction of its length.
	 * @note A split arbitrarily close to an endpoint produces a sliver face and two near-coincident vertices, which
	 *       reads as a click that did nothing and leaves geometry that is awkward to pick apart afterwards. Clamping
	 *       costs a click near a corner its exact position; the alternative costs it a usable result.
	 */
	static constexpr double EndpointClearance = 0.05;
};

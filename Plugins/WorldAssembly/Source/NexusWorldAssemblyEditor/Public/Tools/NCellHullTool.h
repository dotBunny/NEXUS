// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "Tools/NCellToolBase.h"

#include "NCellHullTool.generated.h"

class UCombinedTransformGizmo;
class UTransformProxy;

/** Builds UNCellHullTool for the edit mode's focused cell. */
UCLASS()
class NEXUSWORLDASSEMBLYEDITOR_API UNCellHullToolBuilder : public UNCellToolBuilderBase
{
	GENERATED_BODY()

public:
	//~UInteractiveToolBuilder
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
	//End UInteractiveToolBuilder
};

/**
 * Edits the focused cell's convex hull: click a vertex to drag it, or an edge to select it for splitting.
 *
 * Replaces the hull half of FNCellRootComponentVisualizer's hit-proxy editing. The convexity gate it inherits is the
 * substantive part — a drag that would make a hull non-convex is reverted through NCellHull::SetVertex rather than by
 * writing the vertex array directly, so the cached face planes and convexity verdict describe the hull that survived.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
UCLASS()
class NEXUSWORLDASSEMBLYEDITOR_API UNCellHullTool : public UNCellToolBase, public IClickBehaviorTarget
{
	GENERATED_BODY()

public:
	//~UInteractiveTool
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	virtual void OnTick(float DeltaTime) override;
	//End UInteractiveTool

	//~IClickBehaviorTarget
	virtual FInputRayHit IsHitByClick(const FInputDeviceRay& ClickPos) override;
	virtual void OnClicked(const FInputDeviceRay& ClickPos) override;
	//End IClickBehaviorTarget

	/** @return true when a full edge is selected and can be split. */
	bool HasEdgeSelected() const { return EdgeStartIndex != INDEX_NONE && EdgeEndIndex != INDEX_NONE; }

	/** Split the selected edge and retriangulate, then clear the selection. */
	void SplitSelectedEdge();

protected:
	//~UNCellToolBase
	virtual TOptional<UNWorldAssemblyEdMode::ENCellEdMode> GetCellEdMode() const override { return UNWorldAssemblyEdMode::ENCellEdMode::Hull; }
	//End UNCellToolBase

private:
	/** Drop any vertex/edge selection and tear down the drag gizmo. */
	void ClearSelection();

	/** Create (or move) the translate gizmo onto the selected vertex. */
	void UpdateVertexGizmo();

	/** Write a dragged vertex back onto the hull, reverting it when that would break a convexity requirement. */
	void OnVertexTransformChanged(UTransformProxy* Proxy, FTransform Transform);

	/** Open the drag's undo transaction. */
	void OnVertexTransformBegin(UTransformProxy* Proxy);

	/** Close the drag's undo transaction. */
	void OnVertexTransformEnd(UTransformProxy* Proxy);

	/**
	 * @param Ray Click ray in world space.
	 * @param OutDistance Distance along Ray to the winning vertex, untouched when nothing is picked.
	 * @return Index of the hull vertex nearest the ray within the pick threshold, or INDEX_NONE.
	 */
	int32 FindVertexUnderRay(const FRay& Ray, double& OutDistance) const;

	/**
	 * @param Ray Click ray in world space.
	 * @param OutDistance Distance along Ray to the winning edge, untouched when nothing is picked.
	 * @return Index into the cached edge array nearest the ray within the pick threshold, or INDEX_NONE.
	 */
	int32 FindEdgeUnderRay(const FRay& Ray, double& OutDistance) const;

	UPROPERTY()
	TObjectPtr<UTransformProxy> VertexProxy;

	UPROPERTY()
	TObjectPtr<UCombinedTransformGizmo> VertexGizmo;

	/** Index of the hull vertex being dragged, or INDEX_NONE. */
	int32 VertexIndex = INDEX_NONE;

	/** Endpoints of the selected edge, or INDEX_NONE each. Mutually exclusive with a vertex selection. */
	int32 EdgeStartIndex = INDEX_NONE;
	int32 EdgeEndIndex = INDEX_NONE;

	/**
	 * Open while a vertex is being dragged; scopes the whole drag into one undo entry.
	 * @note Also what tells OnTick a drag is in progress, so it leaves the gizmo alone.
	 */
	TUniquePtr<FScopedTransaction> DragTransaction;
};

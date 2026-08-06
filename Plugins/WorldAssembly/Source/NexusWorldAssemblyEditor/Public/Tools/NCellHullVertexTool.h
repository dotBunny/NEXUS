// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "Tools/NCellToolBase.h"

#include "NCellHullVertexTool.generated.h"

class UCombinedTransformGizmo;
class UTransformProxy;

/** Builds UNCellHullVertexTool for the edit mode's focused cell. */
UCLASS()
class NEXUSWORLDASSEMBLYEDITOR_API UNCellHullVertexToolBuilder : public UNCellToolBuilderBase
{
	GENERATED_BODY()

public:
	//~UInteractiveToolBuilder
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
	//End UInteractiveToolBuilder
};

/**
 * Edits the focused cell's convex hull: click a vertex to drag it, shift- or ctrl-click to drag several together.
 *
 * Replaces the hull half of FNCellRootComponentVisualizer's hit-proxy editing. The convexity gate it inherits is the
 * substantive part — a drag that would make a hull non-convex is reverted through NCellHull::SetVertex rather than by
 * writing the vertex array directly, so the cached face planes and convexity verdict describe the hull that survived.
 * With several vertices selected the gate judges the move as a whole and reverts all of them together, because a
 * multi-vertex move is one edit and half of it applied is a shape the user never asked for.
 *
 * @note Adding vertices is UNCellHullSplitTool's job, not this one's. This tool used to carry an edge selection for
 *       the Split Hull Edge action to act on, which is the arrangement that tool replaced.
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
UCLASS()
class NEXUSWORLDASSEMBLYEDITOR_API UNCellHullVertexTool : public UNCellToolBase, public IClickBehaviorTarget
{
	GENERATED_BODY()

public:
	//~UInteractiveTool
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	virtual void OnTick(float DeltaTime) override;

	/**
	 * Draw a handle on every hull vertex, highlighting the selected ones.
	 * @note Here rather than left to FNCellRootComponentVisualizer, which draws the same points but only runs for a
	 *       selected actor — and the tool targets the mode's focused cell whether or not it is selected. Without this
	 *       the vertices this tool exists to drag are invisible until one has been clicked, and clicking one means
	 *       already knowing where it is.
	 * @remark The highlight is not decoration once a selection can hold more than one vertex: the gizmo sits at their
	 *         midpoint, which for a spread-out selection is nowhere near any of them.
	 */
	virtual void Render(IToolsContextRenderAPI* RenderAPI) override;
	//End UInteractiveTool

	//~IClickBehaviorTarget
	virtual FInputRayHit IsHitByClick(const FInputDeviceRay& ClickPos) override;
	virtual void OnClicked(const FInputDeviceRay& ClickPos) override;

	/** Tracks the extend-selection modifiers registered in Setup. */
	virtual void OnUpdateModifierState(int ModifierID, bool bIsOn) override;
	//End IClickBehaviorTarget

protected:
	//~UNCellToolBase
	virtual TOptional<UNWorldAssemblyEdMode::ENCellEdMode> GetCellEdMode() const override { return UNWorldAssemblyEdMode::ENCellEdMode::Hull; }
	//End UNCellToolBase

private:
	/** Modifier id for shift, registered on the click behavior. */
	static constexpr int32 ShiftModifierID = 1;

	/** Modifier id for ctrl, registered on the click behavior. */
	static constexpr int32 CtrlModifierID = 2;

	/** Drop the vertex selection and tear down the drag gizmo. */
	void ClearSelection();

	/** Rebuild the translate gizmo at the midpoint of the current selection, or tear it down when nothing is selected. */
	void UpdateVertexGizmo();

	/** @return Midpoint of the selected vertices in world space, which is where the gizmo sits. */
	FVector GetSelectionCenter() const;

	/** Write the drag's translation onto every selected vertex, reverting them all when that breaks convexity. */
	void OnVertexTransformChanged(UTransformProxy* Proxy, FTransform Transform);

	/** Open the drag's undo transaction and snapshot what the drag is moving. */
	void OnVertexTransformBegin(UTransformProxy* Proxy);

	/** Close the drag's undo transaction and drop the snapshot. */
	void OnVertexTransformEnd(UTransformProxy* Proxy);

	/**
	 * @param Ray Click ray in world space.
	 * @param OutDistance Distance along Ray to the winning vertex, untouched when nothing is picked.
	 * @return Index of the hull vertex nearest the ray within the pick threshold, or INDEX_NONE.
	 */
	int32 FindVertexUnderRay(const FRay& Ray, double& OutDistance) const;

	UPROPERTY()
	TObjectPtr<UTransformProxy> VertexProxy;

	UPROPERTY()
	TObjectPtr<UCombinedTransformGizmo> VertexGizmo;

	/** Indices of the selected hull vertices, in the order they were clicked. Empty when nothing is selected. */
	TArray<int32> VertexIndices;

	/**
	 * Local-space positions of the selected vertices as they were when the drag began, parallel to VertexIndices.
	 * @note A drag has to be applied as a delta from where things started, not as an absolute position: the gizmo
	 *       reports one transform for the whole selection, and reading the current vertex positions each frame would
	 *       accumulate the delta on top of itself. Non-empty only between OnVertexTransformBegin and its End.
	 */
	TArray<FVector> DragStartPositions;

	/** Gizmo location when the drag began; every frame's translation is measured from here. */
	FVector DragStartCenter = FVector::ZeroVector;

	/**
	 * Open while the selection is being dragged; scopes the whole drag into one undo entry.
	 * @note Also what tells OnTick a drag is in progress, so it leaves the gizmo alone.
	 */
	TUniquePtr<FScopedTransaction> DragTransaction;

	/** True while shift is held, per the click behavior's modifier reporting. */
	bool bShiftDown = false;

	/** True while ctrl is held, per the click behavior's modifier reporting. */
	bool bCtrlDown = false;

	/**
	 * Pixel size of a drawn vertex handle.
	 * @note Matches FNCellRootComponentVisualizer's, so a hull vertex looks the same whichever of the two drew it. It
	 *       also lands near the pick radius IsPickedByRay resolves to on a typical viewport, which is what makes the
	 *       dot a fair advertisement of what clicking near it will hit.
	 */
	static constexpr float VertexPointSize = 12.0f;

	/** Pixel size of a selected vertex handle. Larger so a selection reads at a glance, not only by its color. */
	static constexpr float SelectedVertexPointSize = 16.0f;
};

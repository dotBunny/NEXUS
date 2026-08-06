// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Tools/NCellToolBase.h"

#include "NCellBoundsTool.generated.h"

class UCombinedTransformGizmo;
class UTransformProxy;

/** Builds UNCellBoundsTool for the edit mode's focused cell. */
UCLASS()
class NEXUSWORLDASSEMBLYEDITOR_API UNCellBoundsToolBuilder : public UNCellToolBuilderBase
{
	GENERATED_BODY()

public:
	//~UInteractiveToolBuilder
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
	//End UInteractiveToolBuilder
};

/**
 * Edits the focused cell's axis-aligned bounds by dragging their min and max corners.
 *
 * Replaces the bounds half of FNCellRootComponentVisualizer's hit-proxy editing: rather than clicking a drawn point
 * to arm the legacy transform widget, each corner carries its own translate gizmo for as long as the tool runs.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
UCLASS()
class NEXUSWORLDASSEMBLYEDITOR_API UNCellBoundsTool : public UNCellToolBase
{
	GENERATED_BODY()

public:
	//~UInteractiveTool
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	virtual void OnTick(float DeltaTime) override;
	//End UInteractiveTool

protected:
	//~UNCellToolBase
	virtual TOptional<UNWorldAssemblyEdMode::ENCellEdMode> GetCellEdMode() const override { return UNWorldAssemblyEdMode::ENCellEdMode::Bounds; }
	//End UNCellToolBase

private:
	/** Which corner of the bounds a gizmo drives. */
	enum class ENCorner : uint8
	{
		Min,
		Max
	};

	/** Spawn the translate gizmo for one corner and wire its proxy back to the bounds. */
	void CreateCornerGizmo(ENCorner Corner);

	/** Apply a gizmo's new world transform to the corner it drives. */
	void OnCornerTransformChanged(UTransformProxy* Proxy, FTransform Transform, ENCorner Corner);

	/** Open the drag's undo transaction. Called once per drag rather than per mouse-move frame. */
	void OnCornerTransformBegin(UTransformProxy* Proxy);

	/** Close the drag's undo transaction. */
	void OnCornerTransformEnd(UTransformProxy* Proxy);

	/** Push the current bounds back onto the gizmos, so external edits (undo, Calculate Bounds) move them. */
	void SyncGizmosToBounds();

	/** Move one corner's gizmo onto the bounds, if it isn't already there. */
	void SyncCornerGizmo(UCombinedTransformGizmo* Gizmo, ENCorner Corner) const;

	/** @return The world-space position of Corner for the focused cell, honoring the cell root's offset transform. */
	FVector GetCornerWorldPosition(ENCorner Corner) const;

	UPROPERTY()
	TObjectPtr<UTransformProxy> MinProxy;

	UPROPERTY()
	TObjectPtr<UTransformProxy> MaxProxy;

	UPROPERTY()
	TObjectPtr<UCombinedTransformGizmo> MinGizmo;

	UPROPERTY()
	TObjectPtr<UCombinedTransformGizmo> MaxGizmo;

	/**
	 * Open while a corner is being dragged; scopes the whole drag into one undo entry.
	 * @note Also what tells SyncGizmosToBounds a drag is in progress, so it leaves the gizmos alone.
	 */
	TUniquePtr<FScopedTransaction> DragTransaction;
};

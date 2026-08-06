// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "InteractiveTool.h"
#include "InteractiveToolBuilder.h"
#include "NWorldAssemblyEdMode.h"

#include "NCellToolBase.generated.h"

class UNCellRootComponent;

/**
 * Shared builder for the cell-authoring tools, which all target the edit mode's focused cell actor.
 *
 * @note The target is the mode's focused cell rather than the editor selection: a level holds at most one
 *       ANCellActor and the mode resolves it every tick, so requiring the user to select it first would be
 *       ceremony. This is why the builder ignores FToolBuilderState's selection entirely.
 */
UCLASS(Abstract)
class NEXUSWORLDASSEMBLYEDITOR_API UNCellToolBuilderBase : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:
	//~UInteractiveToolBuilder
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	//End UInteractiveToolBuilder
};

/**
 * Shared base for the cell-authoring tools.
 *
 * Resolves and holds the focused cell's root component for the life of the tool, and drives the mode's cell-edit
 * sub-mode so the existing overlay drawing (UNWorldAssemblyEdMode's cache refresh, and FNCellRootComponentVisualizer's
 * point rendering) follows whichever tool is running.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
UCLASS(Abstract)
class NEXUSWORLDASSEMBLYEDITOR_API UNCellToolBase : public UInteractiveTool
{
	GENERATED_BODY()

public:
	//~UInteractiveTool
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;

	/** @return Always false: these tools commit as they go, so there is nothing to accept. */
	virtual bool HasAccept() const override { return false; }
	virtual bool CanAccept() const override { return false; }

	/** @return Always false: every edit is already its own transaction, so Cancel would have nothing to roll back. */
	virtual bool HasCancel() const override { return false; }
	//End UInteractiveTool

protected:
	/**
	 * @return The cell-edit sub-mode this tool puts the edit mode into while it runs, or unset to leave it alone.
	 * @note Unset is the right answer for a tool whose geometry is not one of the three the sub-mode selects between —
	 *       junction placement, for instance, wants whatever overlay the user already had up.
	 */
	virtual TOptional<UNWorldAssemblyEdMode::ENCellEdMode> GetCellEdMode() const { return TOptional<UNWorldAssemblyEdMode::ENCellEdMode>(); }

	/** @return The focused cell's root component, or nullptr if the cell went away while the tool was running. */
	UNCellRootComponent* GetCellRoot() const;

	/** Flag the focused cell actor dirty after a mutation. Safe to call when the cell has gone. */
	void MarkCellDirty() const;

	/**
	 * Tests a world-space point against a click ray for picking.
	 *
	 * @param Ray Click ray in world space.
	 * @param Point Candidate point to test.
	 * @param OutRayParameter Distance along Ray of the closest approach to Point. Doubles as the pick depth, so
	 *        callers can prefer the nearest of several hits.
	 * @return true when Point is close enough to Ray to count as picked.
	 * @note The threshold widens with distance along the ray, which keeps the pick target roughly constant on screen
	 *       — the behavior the hit proxies this replaced got for free by being rendered.
	 */
	static bool IsPickedByRay(const FRay& Ray, const FVector& Point, double& OutRayParameter);

	/** The cell actor the tool was started against. Weak: the user can delete it without leaving the tool. */
	UPROPERTY()
	TWeakObjectPtr<ANCellActor> CellActor;
};

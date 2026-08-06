// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "Tools/NCellToolBase.h"

#include "NCellVoxelTool.generated.h"

/** Builds UNCellVoxelTool for the edit mode's focused cell. */
UCLASS()
class NEXUSWORLDASSEMBLYEDITOR_API UNCellVoxelToolBuilder : public UNCellToolBuilderBase
{
	GENERATED_BODY()

public:
	//~UInteractiveToolBuilder
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
	//End UInteractiveToolBuilder
};

/**
 * Toggles individual voxels of the focused cell between occupied and empty.
 *
 * Replaces the voxel half of FNCellRootComponentVisualizer's hit-proxy editing. Each toggle is its own transaction,
 * as it was before, and turns off the cell's calculate-on-save so the save pass does not overwrite the hand edit.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
UCLASS()
class NEXUSWORLDASSEMBLYEDITOR_API UNCellVoxelTool : public UNCellToolBase, public IClickBehaviorTarget
{
	GENERATED_BODY()

public:
	//~UInteractiveTool
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	//End UInteractiveTool

	//~IClickBehaviorTarget
	virtual FInputRayHit IsHitByClick(const FInputDeviceRay& ClickPos) override;
	virtual void OnClicked(const FInputDeviceRay& ClickPos) override;
	//End IClickBehaviorTarget

protected:
	//~UNCellToolBase
	virtual TOptional<UNWorldAssemblyEdMode::ENCellEdMode> GetCellEdMode() const override { return UNWorldAssemblyEdMode::ENCellEdMode::Voxel; }
	//End UNCellToolBase

private:
	/**
	 * @param Ray Click ray in world space.
	 * @param OutDistance Distance along Ray to the winning voxel, untouched when nothing is picked.
	 * @return Flat index of the voxel nearest the ray within the pick threshold, or INDEX_NONE.
	 */
	int32 FindVoxelUnderRay(const FRay& Ray, double& OutDistance) const;

	/** Flip the voxel at Index between occupied and empty, in its own transaction. */
	void ToggleVoxel(int32 Index) const;
};

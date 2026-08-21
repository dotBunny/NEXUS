// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NEdModeRail.h"

class FBindingContext;

/**
 * The Cell category: the tools and calculations that author the focused cell.
 *
 * Owns the declaration, binding and gating for everything that acts on the focused cell. The work itself lives in
 * FNWorldAssemblyEditorCellUtils, so this stays a description of what the UI offers rather than of what the cell
 * does. Commands are declared against the module's shared binding context by FNWorldAssemblyEditorCommands, which is
 * the only TCommands in the module — every category registers into it so they share one Keyboard Shortcuts section.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNCellEdModeRail final : public FNEdModeRail
{
public:
	using FNEdModeRail::FNEdModeRail;

	/**
	 * Declare every cell command against the module's binding context and map each to its action.
	 * @param Context The module's shared FBindingContext, supplied by FNWorldAssemblyEditorCommands::RegisterCommands.
	 */
	static void RegisterCommands(const TSharedRef<FBindingContext>& Context);

	/** @return This category's commands, mapped to their actions, for the toolkit to fold into its own list. */
	static TSharedRef<FUICommandList> GetCommandList();

	/** Select the cell actor that owns the current cell-editor context. */
	static void SelectActor();
	/** @return true if a cell-actor selection target is available. */
	static bool SelectActor_CanExecute();

	/**
	 * @return true if cell data can be recalculated right now.
	 * @note FNWorldAssemblyEditorUtils::CanEditCell plus a settled terrain. Recalculating while a terrain build is
	 *       still landing sections bakes data describing a half-built world, so the commands go unavailable until it
	 *       finishes rather than producing a result that quietly disagrees with the one the same button gives a
	 *       moment later.
	 */
	static bool Calculate_CanExecute();

	/** @return true if voxel recalculation is possible (e.g. bounds exist). */
	static bool CalculateVoxelData_CanExecute();

	/** Add or remove the cell-ignore tag across the current actor selection. */
	static void TagIgnore();
	/**
	 * @return The tag icon while the next click would add the tag, and the remove-tag icon once it would take it off.
	 * @note Bound as the rail button's icon rather than resolved once, so it tracks the selection changing under it.
	 */
	static FSlateIcon TagIgnoreIcon();
	/** @return true if the cell-ignore tagging action can act on the current selection. */
	static bool TagIgnore_CanExecute();

	/** Cycle the viewport overlay's voxel drawing through grid, points, and off. */
	static void ToggleDrawVoxelData();
	/** @return checked state of the draw-voxel-data toggle for UI binding. */
	static bool ToggleDrawVoxelData_IsActionChecked();

	//~FNEdModeRail
	virtual TSharedPtr<FUICommandInfo> GetCategoryCommand() const override;
	virtual TAttribute<bool> GetAvailable() const override;
	virtual bool ShouldAutoSelect() const override;
	virtual TSharedPtr<SWidget> CreateContent() const override;
	//End FNEdModeRail

private:
	/**
	 * This category's own command list, built once at module startup.
	 * @note Static, unlike the base's CommandList: that one is the toolkit's, handed to a rail instance that is rebuilt
	 *       every time the edit mode opens. The commands themselves outlive any of that — they are declared against a
	 *       module-lifetime binding context, which is what keeps their chords in Editor Preferences.
	 */
	static TSharedPtr<FUICommandList> CategoryCommandList;

	static TSharedPtr<FUICommandInfo> CommandInfo_SelectActor;
	static TSharedPtr<FUICommandInfo> CommandInfo_TagIgnore;

	static TSharedPtr<FUICommandInfo> CommandInfo_CalculateAll;
	static TSharedPtr<FUICommandInfo> CommandInfo_CalculateBounds;
	static TSharedPtr<FUICommandInfo> CommandInfo_CalculateHull;
	static TSharedPtr<FUICommandInfo> CommandInfo_CalculateVoxelData;

	static TSharedPtr<FUICommandInfo> CommandInfo_ToggleDrawVoxelData;
	static TSharedPtr<FUICommandInfo> CommandInfo_ToggleBoundsCalculateOnSave;
	static TSharedPtr<FUICommandInfo> CommandInfo_ToggleHullCalculateOnSave;
	static TSharedPtr<FUICommandInfo> CommandInfo_ToggleHullAllowNonConvex;
	static TSharedPtr<FUICommandInfo> CommandInfo_ToggleVoxelCalculateOnSave;
	static TSharedPtr<FUICommandInfo> CommandInfo_ToggleVoxelData;
};

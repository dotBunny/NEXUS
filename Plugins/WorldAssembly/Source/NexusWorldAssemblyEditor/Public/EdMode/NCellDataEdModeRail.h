// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEdModeRail.h"

class FBindingContext;

/**
 * The Cell Data category: the focused cell's side-car asset, and the actor that owns it.
 *
 * Split off from FNWorldAssemblyEdModeCellRail, which authors the cell's geometry: everything here acts on the asset
 * or on the actor's existence instead. Two groups of one-shot actions — the actor's, then the data's. The work itself
 * lives in FNWorldAssemblyEditorCellUtils.
 *
 * @note Add Cell Actor is not here but on FNWorldAssemblyEdModeWorldRail: this category is off the rail until the
 *       level has a cell, so the command that puts one there cannot be the one that needs it to already exist.
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNCellDataEdModeRail final : public FNEdModeRail
{
public:
	using FNEdModeRail::FNEdModeRail;

	/**
	 * Declare every cell data command against the module's binding context and map each to its action.
	 * @param Context The module's shared FBindingContext, supplied by FNWorldAssemblyEditorCommands::RegisterCommands.
	 */
	static void RegisterCommands(const TSharedRef<FBindingContext>& Context);

	/** @return This category's commands, mapped to their actions, for the toolkit to fold into its own list. */
	static TSharedRef<FUICommandList> GetCommandList();

	/** Save the focused cell to its side-car package. */
	static void SaveCell();

	/** @return true if the active viewport is suitable for thumbnail capture. */
	static bool CaptureThumbnail_CanExecute();

	//~FNWorldAssemblyEdModeRail
	virtual TSharedPtr<FUICommandInfo> GetCategoryCommand() const override;

	/**
	 * @return A predicate requiring a cell actor in the level.
	 * @note Every tool here refuses to build without one (UNCellToolBuilderBase::CanBuildTool) and every action needs
	 *       one to act on. Add Cell Actor deliberately lives on the World rail, which is never hidden, so the cell that
	 *       brings this category back is still reachable while it is gone.
	 */
	virtual TAttribute<bool> GetAvailable() const override;

	virtual TSharedPtr<SWidget> CreateContent() const override;
	//End FNWorldAssemblyEdModeRail

private:
	/**
	 * This category's own command list, built once at module startup.
	 * @note Static, unlike the base's CommandList: that one is the toolkit's, handed to a rail instance that is rebuilt
	 *       every time the edit mode opens. The commands themselves outlive any of that — they are declared against a
	 *       module-lifetime binding context, which is what keeps their chords in Editor Preferences.
	 */
	static TSharedPtr<FUICommandList> CategoryCommandList;

	static TSharedPtr<FUICommandInfo> CommandInfo_RemoveActor;

	static TSharedPtr<FUICommandInfo> CommandInfo_CaptureThumbnail;

	static TSharedPtr<FUICommandInfo> CommandInfo_ResetCell;
	static TSharedPtr<FUICommandInfo> CommandInfo_SaveCell;
};

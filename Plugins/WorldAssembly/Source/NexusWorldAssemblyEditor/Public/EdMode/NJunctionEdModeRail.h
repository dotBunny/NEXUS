// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NEdModeRail.h"

class FBindingContext;

/**
 * The Junction category: placing and managing the focused cell's junctions.
 *
 * @see <a href="https://nexus-framework.com/docs/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNJunctionEdModeRail final : public FNEdModeRail
{
public:
	using FNEdModeRail::FNEdModeRail;

	/**
	 * Declare every junction command against the module's binding context and map each to its action.
	 * @param Context The module's shared FBindingContext, supplied by FNWorldAssemblyEditorCommands::RegisterCommands.
	 */
	static void RegisterCommands(const TSharedRef<FBindingContext>& Context);

	/** @return This category's commands, mapped to their actions, for the toolkit to fold into its own list. */
	static TSharedRef<FUICommandList> GetCommandList();

	/** @return true when the focused level has a cell actor, actors are selected, and we are not in PIE. */
	static bool CanEditCellJunction();

	//~FNEdModeRail
	virtual TSharedPtr<FUICommandInfo> GetCategoryCommand() const override;

	/** @return A predicate requiring a cell actor; a junction is a component of one, so nothing here works without it. */
	virtual TAttribute<bool> GetAvailable() const override;

	/**
	 * @return The tools, then the component operations, then the junction picker.
	 * @note The picker is built as part of the content rather than handed to the panel separately, which is what puts the ordering in the
	 *       rail's hands, and it costs nothing — the panel drops both into the same scrolling column anyway, so the
	 *       picker only has to carry the inset the header slot would have given it.
	 */
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

	static TSharedPtr<FUICommandInfo> CommandInfo_AddComponent;
	static TSharedPtr<FUICommandInfo> CommandInfo_CollectComponents;
};

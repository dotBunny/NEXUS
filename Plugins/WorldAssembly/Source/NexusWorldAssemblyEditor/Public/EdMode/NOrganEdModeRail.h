// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NEdModeRail.h"

class FBindingContext;

/**
 * The Organ category: generating an organ's cell proxies and managing their level instances.
 *
 * @note Every command here comes in a pair — one acting on the organ the picker names, one on every organ in the
 *       level — so the palette is split along that line rather than by what the commands do.
 * @note Add Organ Volume is not here but on FNWorldEdModeRail: this category is off the rail until the
 *       level has an organ, so the command that puts one there cannot be the one that needs it to already exist.
 * @see <a href="https://nexus-framework.com/docs/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNOrganEdModeRail final : public FNEdModeRail
{
public:
	using FNEdModeRail::FNEdModeRail;

	/**
	 * Declare every organ command against the module's binding context and map each to its action.
	 * @param Context The module's shared FBindingContext, supplied by FNWorldAssemblyEditorCommands::RegisterCommands.
	 */
	static void RegisterCommands(const TSharedRef<FBindingContext>& Context);

	/** @return This category's commands, mapped to their actions, for the toolkit to fold into its own list. */
	static TSharedRef<FUICommandList> GetCommandList();

	//~FNEdModeRail
	virtual TSharedPtr<FUICommandInfo> GetCategoryCommand() const override;

	/** @return A predicate requiring an organ in the level; Add Organ Volume on the World rail is what brings this back. */
	virtual TAttribute<bool> GetAvailable() const override;

	/** @return Always true: a level holding organs is a level assembled from them, so the mode opens here. */
	virtual bool ShouldAutoSelect() const override;

	/**
	 * @return The world-wide commands, then the Selected Organ section — the organ picker over the commands acting on
	 *         what it names — then the operations list.
	 * @note The picker is built as part of the content rather than handed to the panel separately, so it leads the one
	 *       section whose commands it decides the target of instead of floating over the whole category.
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

	static TSharedPtr<FUICommandInfo> CommandInfo_GenerateProxies;
	static TSharedPtr<FUICommandInfo> CommandInfo_GenerateAllProxies;
	static TSharedPtr<FUICommandInfo> CommandInfo_ClearProxies;
	static TSharedPtr<FUICommandInfo> CommandInfo_ClearAllProxies;
	static TSharedPtr<FUICommandInfo> CommandInfo_LoadLevelInstances;
	static TSharedPtr<FUICommandInfo> CommandInfo_CreateAllLevelInstances;
	static TSharedPtr<FUICommandInfo> CommandInfo_UnloadLevelInstances;
	static TSharedPtr<FUICommandInfo> CommandInfo_UnloadAllLevelInstances;
};

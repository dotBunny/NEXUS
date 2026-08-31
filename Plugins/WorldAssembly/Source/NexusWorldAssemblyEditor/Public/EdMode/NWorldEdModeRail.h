// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEdModeRail.h"

class FBindingContext;

/**
 * The World category: bringing cells and organs into the level, and the world collision they assemble against.
 *
 * Also owns the two commands that create what the other categories edit — placing the cell actor and placing an organ
 * volume. They are declared here rather than beside the rest of their subject's commands because a command belongs to
 * the category that offers it, and this is the only one that can offer these: the Cell and Organ rails are hidden
 * until the level already holds what these place. Only declaration and gating live here — the work stays with its
 * subject, in FNWorldAssemblyEditorCellUtils and FNWorldAssemblyEditorOrganUtils.
 *
 * @note The only rail with no availability predicate, and deliberately so. Every other category leaves the strip
 *       until the thing it edits exists, so this one carries both Add commands — it is what stays reachable in an
 *       empty level, and what FNWorldAssemblyRailState falls back to when the selected category disappears.
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldEdModeRail final : public FNEdModeRail
{
public:
	using FNEdModeRail::FNEdModeRail;

	/**
	 * Declare every world command against the module's binding context and map each to its action.
	 * @param Context The module's shared FBindingContext, supplied by FNWorldAssemblyEditorCommands::RegisterCommands.
	 */
	static void RegisterCommands(const TSharedRef<FBindingContext>& Context);

	/** @return This category's commands, mapped to their actions, for the toolkit to fold into its own list. */
	static TSharedRef<FUICommandList> GetCommandList();

	/** Spawn the world-collision visualizer, or destroy it if one is already alive. */
	static void ToggleCollisionVisualizer();
	/** @return checked state of the collision-visualizer toggle for UI binding. */
	static bool ToggleCollisionVisualizer_IsActionChecked();

	/** @return true if a cell actor can be added: the level has none already, and holds no organs. */
	static bool AddCellActor_CanExecute();

	/** @return true if an organ volume can be added: the level is not itself a cell. */
	static bool AddOrganVolume_CanExecute();

	/**
	 * Bake the world collision the level's organs assemble against into the level's collision cache.
	 *
	 * Acts on the selected organs when any are selected, and on every organ in the level otherwise — so the common
	 * case is one click with nothing selected, and narrowing to a single organ is a matter of selecting it first.
	 * @note Always a forced re-bake, unlike the save-time pass. Someone reaching for this button has a reason to
	 *       distrust what is stored, and a button that silently did nothing because a fingerprint still matched
	 *       would be no way to act on that.
	 */
	static void CacheWorldCollision();

	/** @return true if the level holds an organ to bake and the editor is not in a play session. */
	static bool CacheWorldCollision_CanExecute();

	/** Add or remove the world-collision-ignore tag across the current actor selection. */
	static void TagCollisionIgnore();
	/**
	 * @return The tag icon while the next click would add the tag, and the remove-tag icon once it would take it off.
	 * @note Bound as the rail button's icon rather than resolved once, so it tracks the selection changing under it.
	 */
	static FSlateIcon TagCollisionIgnoreIcon();
	/** @return true if the world-collision-ignore tagging action can act on the current selection. */
	static bool TagCollisionIgnore_CanExecute();

	//~FNEdModeRail
	virtual TSharedPtr<FUICommandInfo> GetCategoryCommand() const override;
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

	static TSharedPtr<FUICommandInfo> CommandInfo_ToggleCollisionVisualizer;
	static TSharedPtr<FUICommandInfo> CommandInfo_CacheWorldCollision;

	static TSharedPtr<FUICommandInfo> CommandInfo_AddCellActor;
	static TSharedPtr<FUICommandInfo> CommandInfo_AddOrganVolume;

	static TSharedPtr<FUICommandInfo> CommandInfo_TagCollisionIgnore;
};

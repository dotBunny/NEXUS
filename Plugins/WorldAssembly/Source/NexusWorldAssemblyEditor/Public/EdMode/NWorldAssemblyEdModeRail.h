// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Textures/SlateIcon.h"

class FUICommandInfo;
class FUICommandList;
class SWidget;

/**
 * One category on the World Assembly edit mode's toolkit rail.
 *
 * A rail owns everything specific to its category: the command its rail button is built from, whether that button is
 * shown at all, the picker naming what it acts on, and the buttons beneath. SNWorldAssemblyRail owns the shared frame
 * — laying the buttons out, switching content on the active category, the active-tool row and the warning footer — so
 * a rail never has to know how it is being presented.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEdModeRail
{
public:
	/**
	 * A command in one of the toolbar-backed groups, optionally drawn with an icon of its own.
	 *
	 * @note Converts implicitly from a bare command, so a group wanting nothing special still reads as a braced list
	 *       of command infos. Only the grid and the list take these — the tiles and the check lists draw their icons
	 *       through widgets that have no override to offer.
	 */
	struct FNRailCommand
	{
		FNRailCommand(const TSharedPtr<FUICommandInfo>& InCommand) : Command(InCommand) {}
		FNRailCommand(const TSharedPtr<FUICommandInfo>& InCommand, const TAttribute<FSlateIcon>& InIcon)
			: Command(InCommand), Icon(InIcon) {}

		/** The command the button runs, and takes its label and tooltip from. */
		TSharedPtr<FUICommandInfo> Command;

		/**
		 * Icon drawn instead of the one the command was registered with, or unset to keep that one.
		 * @note Re-read every paint by SToolBarButtonBlock, so bind it to report state rather than resolving an icon
		 *       once here. The registered icon still stands everywhere else the command appears.
		 */
		TAttribute<FSlateIcon> Icon;
	};

	/**
	 * @param InCommandList The toolkit's command list, which every button this rail builds resolves against.
	 */
	explicit FNWorldAssemblyEdModeRail(const TSharedRef<FUICommandList>& InCommandList) : CommandList(InCommandList) {}

	virtual ~FNWorldAssemblyEdModeRail() = default;

	/** @return The command this category's rail button is built from. */
	virtual TSharedPtr<FUICommandInfo> GetCategoryCommand() const = 0;

	/**
	 * @return Predicate deciding whether this category appears on the rail at all, or unset to always show it.
	 * @note Asks "is this category relevant to the level", not "can the user act right now" — a distinction that
	 *       matters because the answer drives visibility. Cell and Junction key off the level containing a cell
	 *       actor, not off one being focused: focus comes and goes with every selection change, and a button hiding
	 *       on that would vanish constantly and shuffle the ones below it under the cursor. Whether the commands
	 *       inside the category can run is each command's own business, and they already grey themselves out.
	 * @remark Polled on a timer by SNWorldAssemblyRail rather than read per-frame, so it may walk the level — Cell's
	 *         and Junction's do exactly that.
	 */
	virtual TAttribute<bool> GetAvailable() const { return TAttribute<bool>(); }

	/**
	 * @return true when a level this category is available in is a level *about* this category, so the mode should open
	 *         on it rather than on the first category it finds.
	 * @note Read once, when FNWorldAssemblyRailState seeds itself as the mode opens — not polled like GetAvailable.
	 *       Whatever the user picks afterwards stands for the rest of the session, however the level changes under it.
	 * @note Only consulted for a category that is available, so one cannot be opened on while its button is hidden.
	 *       Where several say yes, rail order settles it.
	 */
	virtual bool ShouldAutoSelect() const { return false; }

	/** @return This category's own content — its groups of buttons — or null for a category with none. */
	virtual TSharedPtr<SWidget> CreateContent() const { return nullptr; }

protected:
	/**
	 * Build a group of icon tiles, wrapping across the panel.
	 *
	 * @param Commands Commands to lay out, resolved against the toolkit command list.
	 * @return A group of tiles, on a recessed backing.
	 * @note SNCommandTile in an SWrapBox rather than the FUniformToolBarBuilder this used to be. The builder is the
	 *       path FModeToolkit::CreatePaletteWidget takes, so it matched the Landscape and Foliage palettes — including
	 *       their limit, which is that SMultiBoxWidget caps a palette cell at 50 by 43 and PaletteToolBar's label style
	 *       ellipsizes anything wider. Labels of more than about a word were unreadable, and nothing in the style or
	 *       the builder could widen the cell or wrap the label. The tiles keep the style's buttons, label and paddings,
	 *       so the group still reads as one of the engine's.
	 */
	TSharedRef<SWidget> CreateCommandPalette(const TArray<TSharedPtr<FUICommandInfo>>& Commands) const;

	/**
	 * Build a group of half-width command buttons, two to a row.
	 *
	 * @param Commands Commands to lay out, resolved against the toolkit command list.
	 * @param LeadingContent Widget to sit above the buttons in the same group, or null for the buttons alone.
	 *                       For the thing the buttons act on — a picker naming it belongs in the group it decides the
	 *                       target of, not in one of its own.
	 * @return A two-column toolbar widget, drawn straight onto the panel.
	 * @note What the engine's own toolkit builder renders a palette as, reproduced here so a rail can head and split
	 *       one. It is a different widget from CreateCommandPalette's, not a wider setting on it: this is the plugin's
	 *       WorldAssemblyEd.CommandGrid style — SlimPaletteToolBar with its backing cleared — on
	 *       FSlimHorizontalUniformToolBarBuilder, whose SUniformWrapPanel fills its width across the style's two columns.
	 *       Labels sit beside their icons here rather than under them, so a row can carry as much text as half the
	 *       panel is wide.
	 * @remark For commands whose labels are a phrase rather than a name. The tile groups suit a handful of commands the
	 *         user learns by shape; these read as a list of named operations.
	 */
	TSharedRef<SWidget> CreateCommandGrid(const TArray<FNRailCommand>& Commands,
		const TSharedPtr<SWidget>& LeadingContent = nullptr) const;

	/**
	 * Build a group of full-width command buttons, one to a row.
	 *
	 * @param Commands Commands to lay out, resolved against the toolkit command list.
	 * @param LeadingContent Widget to sit above the buttons in the same group, or null for the buttons alone.
	 *                       Same terms as CreateCommandGrid's — for the thing the buttons act on.
	 * @return A single-column toolbar widget.
	 * @note CreateCommandGrid's group at one column instead of two, and without the resting fill its buttons
	 *       carry: a row here is its icon and label until hovered, which is how Mesh Terrain's submode palettes read.
	 *       The grid keeps the fill because two buttons to a row need an edge each to be read as two.
	 * @remark For a short group whose labels are long enough that half a panel ellipsizes them, or one whose commands
	 *         want reading down rather than scanning across.
	 */
	TSharedRef<SWidget> CreateCommandList(const TArray<FNRailCommand>& Commands,
		const TSharedPtr<SWidget>& LeadingContent = nullptr) const;

	/**
	 * Build a group of labelled checkboxes, one per command.
	 *
	 * @param Commands Toggle commands to lay out, resolved against the toolkit command list.
	 * @return A column of checkboxes, drawn straight onto the panel like the command groups.
	 * @note For commands that read as persistent settings rather than actions. The same commands rendered into a
	 *       toolbar become icon buttons whose state is only legible from their highlight, which is the wrong shape
	 *       for something the user is setting rather than doing.
	 * @note A label stays on one line and ellipsizes when the panel is too narrow for it, so the rows keep step with
	 *       each other at any width. The command's description is the tooltip, which is where a shortened one is read.
	 */
	TSharedRef<SWidget> CreateCheckList(const TArray<TSharedPtr<FUICommandInfo>>& Commands) const;

	/**
	 * Put an arbitrary widget in a group of its own, on a recessed backing.
	 *
	 * @param Content The widget to wrap.
	 * @return A group wrapping Content.
	 * @note Where the recessed backing is drawn, for the tile palettes and for a section that is no set of commands at
	 *       all. The labelled-button groups sit straight on the panel, so a well now says the content inside it is not
	 *       one of those — a block of icon tiles, or the Organ rail's operations list.
	 */
	TSharedRef<SWidget> CreateBackedContent(const TSharedRef<SWidget>& Content) const;

	/**
	 * Rule off one run of groups from the next, optionally naming the run that follows.
	 *
	 * @param Label Text to sit at the near end of the rule, with the rule running out from it to the far edge, or
	 *              empty for a bare rule spanning the whole width.
	 * @return A horizontal rule, inset to the same edge the groups around it sit on.
	 * @note The only break there is, now that no group carries a heading — so it earns its place between runs of
	 *       groups that are a different kind of thing from each other, not between every pair of them.
	 * @note A label rides on the rule rather than sitting above it, the way the engine's menus name a section: the
	 *       rule is centered against the text, so the two read as one line with a name in it. Keep them short —
	 *       nothing wraps, and a long one leaves no rule to speak of.
	 * @remark Drop it in its own slot between two group slots. It carries the whole of its own spacing, so the slot
	 *         wants no padding of its own.
	 */
	static TSharedRef<SWidget> CreateGroupSeparator(const FText& Label = FText::GetEmpty());

	/** The toolkit's command list; every button a rail builds resolves its action against this. */
	TSharedRef<FUICommandList> CommandList;
};

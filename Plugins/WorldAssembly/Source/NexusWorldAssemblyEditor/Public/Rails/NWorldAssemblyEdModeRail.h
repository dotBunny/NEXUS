// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

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
	 * @return A widget naming what this category acts on, shown above the content, or null for none.
	 * @note Where the pickers live. They are combo boxes over a list the level decides, so they cannot be built from a
	 *       fixed FUICommandInfo the way every button below them is.
	 */
	virtual TSharedPtr<SWidget> CreateHeader() const { return nullptr; }

	/** @return This category's own content — its titled groups of buttons — or null for a category with none. */
	virtual TSharedPtr<SWidget> CreateContent() const { return nullptr; }

protected:
	/**
	 * Build a headed group of icon tiles, wrapping across the panel.
	 *
	 * @param Title Heading shown above the tiles, or empty for a group that goes without one.
	 * @param Commands Commands to lay out, resolved against the toolkit command list.
	 * @return A titled group of tiles, on the same recessed backing as the groups around it.
	 * @note SNCommandTile in an SWrapBox rather than the FUniformToolBarBuilder this used to be. The builder is the
	 *       path FModeToolkit::CreatePaletteWidget takes, so it matched the Landscape and Foliage palettes — including
	 *       their limit, which is that SMultiBoxWidget caps a palette cell at 50 by 43 and PaletteToolBar's label style
	 *       ellipsizes anything wider. Labels of more than about a word were unreadable, and nothing in the style or
	 *       the builder could widen the cell or wrap the label. The tiles keep the style's buttons, label and paddings,
	 *       so the group still reads as one of the engine's.
	 */
	TSharedRef<SWidget> CreateTitledCommandPalette(const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands) const;

	/**
	 * Build a headed group of half-width command buttons, two to a row.
	 *
	 * @param Title Heading shown above the buttons, or empty for a group that goes without one.
	 * @param Commands Commands to lay out, resolved against the toolkit command list.
	 * @return A titled two-column toolbar widget, on the same recessed backing as CreateTitledCommandPalette's.
	 * @note What the engine's own toolkit builder renders a palette as, reproduced here so a rail can head and split
	 *       one. It is a different widget from CreateTitledCommandPalette's, not a wider setting on it: this is the plugin's
	 *       WorldAssemblyEd.TitledCommandGrid style — SlimPaletteToolBar, recessed — on FSlimHorizontalUniformToolBarBuilder,
	 *       whose SUniformWrapPanel fills its width across the style's two columns. Labels sit beside their icons here
	 *       rather than under them, so a row can carry as much text as half the panel is wide.
	 * @remark For commands whose labels are a phrase rather than a name. The tile groups suit a handful of commands the
	 *         user learns by shape; these read as a list of named operations.
	 */
	TSharedRef<SWidget> CreateTitledCommandGrid(const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands) const;

	/**
	 * Build a headed group of full-width command buttons, one to a row.
	 *
	 * @param Title Heading shown above the buttons, or empty for a group that goes without one.
	 * @param Commands Commands to lay out, resolved against the toolkit command list.
	 * @return A titled single-column toolbar widget.
	 * @note CreateTitledCommandGrid's group at one column instead of two — the same builder, backing and inset, differing
	 *       only in the NumColumns its style carries.
	 * @remark For a short group whose labels are long enough that half a panel ellipsizes them, or one whose commands
	 *         want reading down rather than scanning across.
	 */
	TSharedRef<SWidget> CreateTitledCommandList(const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands) const;

	/**
	 * Build a headed group of labelled checkboxes, one per command.
	 *
	 * @param Title Heading shown above the checkboxes, or empty for a group that goes without one.
	 * @param Commands Toggle commands to lay out, resolved against the toolkit command list.
	 * @return A titled column of checkboxes, on the same recessed backing as the command groups.
	 * @note For commands that read as persistent settings rather than actions. The same commands rendered into a
	 *       toolbar become icon buttons whose state is only legible from their highlight, which is the wrong shape
	 *       for something the user is setting rather than doing.
	 * @remark Built on CreateTitledContent, which is where the heading and backing come from — the checkboxes are the
	 *         only part this adds.
	 */
	TSharedRef<SWidget> CreateTitledCheckList(const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands) const;

	/**
	 * Put an arbitrary widget under a group heading, on the same recessed backing the command groups sit on.
	 *
	 * @param Title Heading shown above the content, or empty for a group that goes without one.
	 * @param Content The widget to head.
	 * @return A titled group wrapping Content.
	 * @note The escape hatch for a section that is not a set of commands. The backing is drawn here rather than coming
	 *       from a toolbar style, since there is no toolbar — same color and inset, so the section reads as a peer of
	 *       the groups around it.
	 */
	TSharedRef<SWidget> CreateTitledContent(const FText& Title, const TSharedRef<SWidget>& Content) const;

	/** The toolkit's command list; every button a rail builds resolves its action against this. */
	TSharedRef<FUICommandList> CommandList;
};

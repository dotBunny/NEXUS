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
 * shown at all, the picker naming what it acts on, and the buttons beneath. FNWorldAssemblyEdModeToolkit owns the
 * shared frame — registering each rail with the builder, gating content on the active category, and the warning
 * footer — so a rail never has to know the builder exists.
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
	 * @return Commands for the category's flat palette toolbar.
	 * @note Empty for a rail that builds its own content instead. FToolPalette renders one flat toolbar with no notion
	 *       of a titled group, so a rail wanting sections returns nothing here and supplies CreateContent instead.
	 */
	virtual TArray<TSharedPtr<FUICommandInfo>> GetPaletteCommands() const { return {}; }

	/**
	 * @return Predicate deciding whether this category's rail button is enabled, or unset to always enable it.
	 * @note Greys the button out rather than hiding it, so the rail keeps a stable shape and a category that is
	 *       unavailable still says so — a button that vanishes leaves nothing to explain why.
	 * @remark Evaluated every frame, so keep it cheap.
	 */
	virtual TAttribute<bool> GetEnabled() const { return TAttribute<bool>(); }

	/**
	 * @return A widget naming what this category acts on, shown above the content, or null for none.
	 * @note Where the pickers live. They cannot be palette buttons: FToolkitBuilder rebinds every button's
	 *       OnGetMenuContent to its own context menu and dereferences the button's FUICommandInfo, so a palette entry
	 *       has to be a fixed command rather than a dynamic list.
	 */
	virtual TSharedPtr<SWidget> CreateHeader() const { return nullptr; }

	/** @return This category's own content, or null when GetPaletteCommands carries it instead. */
	virtual TSharedPtr<SWidget> CreateContent() const { return nullptr; }

protected:
	/**
	 * Build a headed group of command buttons.
	 *
	 * @param Title Heading shown above the buttons.
	 * @param Commands Commands to lay out, resolved against the toolkit command list.
	 * @return A titled toolbar widget.
	 * @note FUniformToolBarBuilder, not the plain one: PaletteToolBar's sizing lives in its Min/MaxUniformToolbarSize
	 *       entries, which only the uniform builder reads. This is the path FModeToolkit::CreatePaletteWidget takes,
	 *       so these groups match the Landscape and Foliage palettes rather than approximating them.
	 */
	TSharedRef<SWidget> CreateTitledCommandPalette(const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands) const;

	/**
	 * Build a headed group of half-width command buttons, two to a row.
	 *
	 * @param Title Heading shown above the buttons.
	 * @param Commands Commands to lay out, resolved against the toolkit command list.
	 * @return A titled two-column toolbar widget, on the same recessed backing as CreateTitledCommandPalette's.
	 * @note What FToolkitBuilder itself renders a palette as, reproduced here so a rail can head and split one. It is a
	 *       different widget from CreateTitledCommandPalette's, not a wider setting on it: this is the plugin's
	 *       WorldAssemblyEd.TitledCommandGrid style — SlimPaletteToolBar, recessed — on FSlimHorizontalUniformToolBarBuilder,
	 *       whose SUniformWrapPanel fills its width across the style's two columns. The other builder's panel is
	 *       left-aligned over fixed 48-unit cells, so it fits as many icon tiles per row as the panel is wide and the
	 *       label under each is ellipsized to nothing useful.
	 * @remark For commands whose labels have to be readable. The icon-tile groups suit a handful of commands the user
	 *         learns by shape; these read as a list of named operations.
	 */
	TSharedRef<SWidget> CreateTitledCommandGrid(const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands) const;

	/**
	 * Build a headed group of full-width command buttons, one to a row.
	 *
	 * @param Title Heading shown above the buttons.
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
	 * @param Title Heading shown above the checkboxes.
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
	 * @param Title Heading shown above the content.
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

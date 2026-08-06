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
	TSharedRef<SWidget> CreateTitledCommandList(const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands) const;

	/**
	 * Build a headed group of labelled checkboxes, one per command.
	 *
	 * @param Title Heading shown above the checkboxes.
	 * @param Commands Toggle commands to lay out, resolved against the toolkit command list.
	 * @return A titled column of checkboxes.
	 * @note For commands that read as persistent settings rather than actions. The same commands rendered into a
	 *       toolbar become icon buttons whose state is only legible from their highlight, which is the wrong shape
	 *       for something the user is setting rather than doing.
	 */
	TSharedRef<SWidget> CreateTitledCheckList(const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands) const;

	/** The toolkit's command list; every button a rail builds resolves its action against this. */
	TSharedRef<FUICommandList> CommandList;
};

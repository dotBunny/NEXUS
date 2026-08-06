// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Rails/NWorldAssemblyEdModeRail.h"
#include "Toolkits/BaseToolkit.h"

/**
 * Toolkit builder for the World Assembly edit mode, laying its content column out inside a scroll box.
 *
 * Exists only to override UpdateContentForCategory. The stock layout puts every section in an AutoHeight slot, so a
 * category whose content outgrows the panel simply overflows — and a scroll box wrapped around the builder's finished
 * widget would take the category rail with it, which has to stay put. Overriding is the seam Epic provides for this;
 * PCG's own toolkit subclasses the builder the same way.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEdModeToolkitBuilder : public FToolkitBuilder
{
public:
	/**
	 * @note Defined out of line, and the class is deliberately not final. Both keep the compiler from instantiating
	 *       FToolkitBuilder's implicit copy constructor here, whose unwind path reaches for
	 *       ~FCategoryDrivenContentBuilderBase — declared but not exported from WidgetRegistration, so it does not
	 *       link from another module. Mirrors how PCG's own toolkit builder is declared.
	 */
	explicit FNWorldAssemblyEdModeToolkitBuilder(FToolkitBuilderArgs& Args);

	/**
	 * @note Deleted rather than merely unused. Leaving them implicit makes the compiler define this class's copy
	 *       constructor, which in turn instantiates FToolkitBuilder's — and that one's unwind path calls
	 *       ~FCategoryDrivenContentBuilderBase, which WidgetRegistration declares but never exports, so it fails to
	 *       link from another module. Nothing copies a builder anyway; it is held by shared pointer.
	 */
	FNWorldAssemblyEdModeToolkitBuilder(const FNWorldAssemblyEdModeToolkitBuilder&) = delete;
	FNWorldAssemblyEdModeToolkitBuilder& operator=(const FNWorldAssemblyEdModeToolkitBuilder&) = delete;

	//~FToolkitBuilder
	virtual void UpdateContentForCategory(FName InActiveCategoryName, FText InActiveCategoryText) override;
	//End FToolkitBuilder

	/**
	 * Supply the widget holding each category's own content, laid out where the palette renders.
	 *
	 * @param InContent Content for every category, each block gating its own visibility on the active one.
	 * @note This is where a category's buttons would live if FToolPalette could express them. It cannot: a palette is
	 *       a flat TArray<FButtonArgs> that CreatePalette pushes into a single toolbar, with no notion of a titled
	 *       group — so any category wanting sections has to render them itself, and this is the slot to do it in.
	 * @remark Must be called before GenerateWidget, which is what triggers the one and only UpdateContentForCategory.
	 */
	void SetCategoryContent(const TSharedRef<SWidget>& InContent) { CategoryContent = InContent; }

	/**
	 * Add a rail category, optionally gated on a predicate that disables its button when false.
	 *
	 * @param Palette The palette to register, exactly as AddPalette takes it.
	 * @param Predicate Evaluated per-frame; false greys the category's rail button out. Unset means always enabled.
	 * @note Wraps AddPalette rather than replacing it. The base maps the load action with an always-true CanExecute
	 *       and adds the button through the FUICommandInfo overload, which resolves enablement through the command
	 *       list — so this copies the action the base built, swaps in the predicate, and re-maps it. Rebuilding the
	 *       action from scratch is not an option: its Execute calls FToolkitBuilder::TogglePalette, which is private.
	 */
	void AddCategory(const TSharedPtr<FToolPalette>& Palette, const TAttribute<bool>& Predicate = TAttribute<bool>());

private:
	/** Per-category content supplied by the toolkit; see SetCategoryContent. */
	TSharedPtr<SWidget> CategoryContent;
};

/**
 * Toolkit for UNWorldAssemblyEdMode, hosting the mode's UI in the level editor's Mode Toolbox panel.
 *
 * Built on FToolkitBuilder (the surface PCG and Modeling Tools use) rather than the stock FModeToolkit palette
 * switcher: a vertical rail of category buttons down the left edge — Cell, Junction, World, Organ — each swapping the
 * palette shown beside it, over a warning area, the active tool's header, and its property details.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEdModeToolkit final : public FModeToolkit
{
public:
	//~FModeToolkit
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode) override;

	/**
	 * @return A name unique to this toolkit.
	 * @note Must not be left at FModeToolkit's "EditorModeToolkit" default. FLevelEditorModeUILayer opens a
	 *       FToolMenuOwnerScoped keyed on this name when it starts hosting a toolkit, and unregisters that owner when
	 *       hosting finishes — so two toolkits sharing a name tear down each other's menu extensions.
	 */
	virtual FName GetToolkitFName() const override { return FName("NWorldAssemblyEdModeToolkit"); }

	virtual FText GetBaseToolkitName() const override;
	virtual TSharedPtr<SWidget> GetInlineContent() const override { return ToolkitWidget; }
	//End FModeToolkit

private:
	/** Construct the rails and register each with the builder. */
	void RegisterPalettes();

	/**
	 * Collect each rail's header — the pickers naming what its buttons act on.
	 *
	 * @return One widget holding every rail's header, each gated on its own category being active.
	 * @note Gathered into FToolkitSections::Header, the one section the builder lays out that takes arbitrary widgets.
	 *       Pickers cannot be palette buttons: FToolkitBuilder rebinds every button's OnGetMenuContent to its own
	 *       context menu and dereferences its FUICommandInfo, so a palette entry has to be a fixed command.
	 */
	TSharedRef<SWidget> CreateContextHeader() const;

	/**
	 * Collect each rail's content, to render where a palette would.
	 *
	 * @return One widget holding every rail's content, each gated on its own category being active.
	 * @note Gating is per block rather than rebuilt on switch because FToolkitBuilder::UpdateContentForCategory runs
	 *       once, from CreateWidget — unlike its sibling FCategoryDrivenContentBuilder, it is not re-invoked when the
	 *       active category changes.
	 */
	TSharedRef<SWidget> CreateCategoryContent() const;

	/**
	 * @return The focused cell's warnings, boxed, collapsing to nothing when there are none.
	 * @note Lives in the footer rather than FToolkitSections::ModeWarningArea: that section sits above the palette,
	 *       so a warning appearing there shifts every button down under the cursor. It is also typed as a bare
	 *       STextBlock, which cannot carry the border.
	 */
	static TSharedRef<SWidget> CreateWarningFooter();

	/**
	 * @param PaletteCommand The rail category command to test.
	 * @return true when that category is the one currently selected on the rail.
	 */
	bool IsPaletteActive(const TSharedPtr<FUICommandInfo>& PaletteCommand) const;

	/**
	 * Wrap a rail's widget so it shows only while that rail's category is active.
	 * @param Widget The rail-supplied widget.
	 * @param CategoryCommand The rail's category command.
	 */
	TSharedRef<SWidget> GateOnCategory(const TSharedRef<SWidget>& Widget, const TSharedPtr<FUICommandInfo>& CategoryCommand) const;

	/** The rail categories, in the order they appear on the rail. */
	TArray<TSharedRef<FNWorldAssemblyEdModeRail>> Rails;
};

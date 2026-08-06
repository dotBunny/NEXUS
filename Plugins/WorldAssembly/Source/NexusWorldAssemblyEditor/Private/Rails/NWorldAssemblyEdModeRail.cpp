// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Rails/NWorldAssemblyEdModeRail.h"

#include "NWorldAssemblyEditorStyle.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/SToolBarButtonBlock.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

/**
 * Mark a widget and every descendant as needing a fresh prepass.
 *
 * @param Widget Root of the subtree to dirty.
 * @note Recursive because a cached desired size is per-widget and Slate's fast path skips any widget whose own
 *       prepass flag is clear — so dirtying only the root leaves the child that actually holds the stale size
 *       untouched. SScaleBox marks its child rather than itself for the same reason; this walks the subtree because
 *       the widget that needs it here is buried inside a multibox we get no handle to.
 */
static void MarkPrepassDirtyRecursive(const TSharedRef<SWidget>& Widget)
{
	Widget->MarkPrepassAsDirty();

	FChildren* Children = Widget->GetAllChildren();
	if (Children == nullptr) return;

	for (int32 Index = 0; Index < Children->Num(); Index++)
	{
		MarkPrepassDirtyRecursive(Children->GetChildAt(Index));
	}
}

/** @return The heading every titled group is topped with, so the four of them stay identical. */
static TSharedRef<SWidget> CreateGroupHeading(const FText& Title)
{
	return SNew(STextBlock)
		.Text(Title)
		.Font(FAppStyle::Get().GetFontStyle("EditorModesPanel.CategoryFontStyle"))
		.ColorAndOpacity(FStyleColors::White25);
}

/**
 * Build a headed group of labelled buttons laid out by one of the plugin's uniform toolbar styles.
 *
 * @param CommandList The toolkit's command list, which every button resolves against.
 * @param StyleName A WorldAssemblyEd.TitledCommand* style; its NumColumns is what decides the layout.
 * @param Title Heading shown above the buttons.
 * @param Commands Commands to lay out.
 * @return A titled toolbar widget.
 */
static TSharedRef<SWidget> CreateTitledUniformToolBar(const TSharedRef<FUICommandList>& CommandList, const FName StyleName,
	const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands)
{
	FSlimHorizontalUniformToolBarBuilder ToolBarBuilder(CommandList, FMultiBoxCustomization::None);
	ToolBarBuilder.SetStyle(&FNWorldAssemblyEditorStyle::Get(), StyleName);

	for (const TSharedPtr<FUICommandInfo>& Command : Commands)
	{
		if (!Command.IsValid()) continue;

		// This builder overrides only the FButtonArgs overload, which hides the plain command one the palette
		// groups use. Leaving UserInterfaceActionType unset would be read as None and silently become a Button, so
		// it is carried across from the command instead — a toggle registered here has to still render as one.
		FButtonArgs ButtonArgs;
		ButtonArgs.Command = Command;
		ButtonArgs.CommandList = CommandList;
		ButtonArgs.UserInterfaceActionType = Command->GetUserInterfaceType();

		ToolBarBuilder.AddToolBarButton(ButtonArgs);
	}

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 10.0f, 8.0f, 2.0f)
		[
			CreateGroupHeading(Title)
		]

		// Same outer inset the palette groups use, so every kind of group lines its backing up down the panel. What
		// separates the buttons from that backing is the style's own BackgroundPadding, not this.
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.Padding(4.0f, 0.0f, 4.0f, 6.0f)
		[
			ToolBarBuilder.MakeWidget()
		];
}

TSharedRef<SWidget> FNWorldAssemblyEdModeRail::CreateTitledCommandPalette(const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands) const
{
	FUniformToolBarBuilder ToolBarBuilder(CommandList, FMultiBoxCustomization::None);
	ToolBarBuilder.SetStyle(&FAppStyle::Get(), "PaletteToolBar");

	for (const TSharedPtr<FUICommandInfo>& Command : Commands)
	{
		ToolBarBuilder.AddToolBarButton(Command);
	}

	const TSharedRef<SWidget> ToolBar = ToolBarBuilder.MakeWidget();

	// One deferred relayout, because SUniformWrapPanel decides its row count from last frame's geometry. The first
	// time this group is laid out it has none, so ComputeDesiredSize falls back to guessing a square — ceil(sqrt(N))
	// columns — which for three or more buttons reserves a row the panel does not need once it knows how wide it
	// really is. The blank row then survives, because nothing recomputes a cached desired size on its own; that is
	// why only resizing the panel clears it, and why it never comes back afterwards.
	//
	// Active timers run from Paint, so this fires on the first frame the group is actually visible rather than while
	// its category is still collapsed — which is exactly when the geometry it needs has just become available.
	//
	// Grid and list groups are immune: their style carries NumColumnsOverride, so the guess is never reached.
	TWeakPtr<SWidget> WeakToolBar = ToolBar;
	ToolBar->RegisterActiveTimer(0.0f, FWidgetActiveTimerDelegate::CreateLambda(
		[WeakToolBar](double, float)
		{
			const TSharedPtr<SWidget> PinnedToolBar = WeakToolBar.Pin();
			if (!PinnedToolBar.IsValid()) return EActiveTimerReturnType::Stop;

			// Nothing useful to recompute against yet — wait for a frame that has real geometry.
			if (PinnedToolBar->GetTickSpaceGeometry().GetLocalSize().IsZero()) return EActiveTimerReturnType::Continue;

			MarkPrepassDirtyRecursive(PinnedToolBar.ToSharedRef());
			PinnedToolBar->Invalidate(EInvalidateWidgetReason::Layout);

			return EActiveTimerReturnType::Stop;
		}));

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 10.0f, 8.0f, 2.0f)
		[
			CreateGroupHeading(Title)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f, 0.0f, 4.0f, 6.0f)
		[
			ToolBar
		];
}

TSharedRef<SWidget> FNWorldAssemblyEdModeRail::CreateTitledCommandGrid(const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands) const
{
	return CreateTitledUniformToolBar(CommandList, "WorldAssemblyEd.TitledCommandGrid", Title, Commands);
}

TSharedRef<SWidget> FNWorldAssemblyEdModeRail::CreateTitledCommandList(const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands) const
{
	return CreateTitledUniformToolBar(CommandList, "WorldAssemblyEd.TitledCommandList", Title, Commands);
}

TSharedRef<SWidget> FNWorldAssemblyEdModeRail::CreateTitledContent(const FText& Title, const TSharedRef<SWidget>& Content) const
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 10.0f, 8.0f, 2.0f)
		[
			CreateGroupHeading(Title)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.Padding(4.0f, 0.0f, 4.0f, 6.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::Get().GetBrush("Brushes.Recessed"))
			// Matches what the toolbar styles carry as their BackgroundPadding, so content lands the same distance
			// inside its backing as a button does inside its.
			.Padding(FMargin(8.0f))
			[
				Content
			]
		];
}

TSharedRef<SWidget> FNWorldAssemblyEdModeRail::CreateTitledCheckList(const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands) const
{
	const TSharedRef<SVerticalBox> Column = SNew(SVerticalBox);

	const TSharedRef<FUICommandList> Commands_CommandList = CommandList;
	for (int32 Index = 0; Index < Commands.Num(); Index++)
	{
		const TSharedPtr<FUICommandInfo>& Command = Commands[Index];
		if (!Command.IsValid()) continue;

		// Captured as a ref so the bindings below do not have to re-check validity every frame.
		const TSharedRef<const FUICommandInfo> CommandRef = Command.ToSharedRef();

		// Spacing between rows only — the backing CreateTitledContent draws carries the group's own inset, and adding
		// to it here would push the checkboxes further off their well than a button sits off its.
		Column->AddSlot()
			.AutoHeight()
			.Padding(0.0f, Index == 0 ? 0.0f : 4.0f, 0.0f, 0.0f)
			[
				SNew(SCheckBox)
				.ToolTipText(Command->GetDescription())
				.IsChecked_Lambda([Commands_CommandList, CommandRef]() { return Commands_CommandList->GetCheckState(CommandRef); })
				.IsEnabled_Lambda([Commands_CommandList, CommandRef]() { return Commands_CommandList->CanExecuteAction(CommandRef); })
				// The command's own Execute already flips the underlying setting, so the new check state is discarded:
				// these are toggles, and driving them from the reported state would double-apply the change.
				.OnCheckStateChanged_Lambda([Commands_CommandList, CommandRef](ECheckBoxState) { Commands_CommandList->ExecuteAction(CommandRef); })
				[
					// SCheckBox sizes itself to its content, so a long label pushes the row past the panel rather than
					// wrapping — the box has no width of its own to wrap against. Filling a slot gives it one: the row
					// is now as wide as the group, and AutoWrapText has a bound to break on.
					SNew(SBox)
					.HAlign(HAlign_Fill)
					[
						SNew(STextBlock)
						.Text(Command->GetLabel())
						.Margin(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
						.AutoWrapText(true)
						// The checkbox label is a name, not a heading — the same dim the group's own heading uses reads
						// as secondary next to the palette buttons rather than competing with them.
						.ColorAndOpacity(FStyleColors::Foreground)
					]
				]
			];
	}

	return CreateTitledContent(Title, Column);
}

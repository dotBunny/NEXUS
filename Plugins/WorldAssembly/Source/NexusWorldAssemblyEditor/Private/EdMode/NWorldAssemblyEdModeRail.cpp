// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "EdMode/NWorldAssemblyEdModeRail.h"

#include "NWorldAssemblyEditorStyle.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/SToolBarButtonBlock.h"
#include "Styling/SlateTypes.h"
#include "Styling/StyleColors.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNCommandTile.h"
#include "Widgets/Text/STextBlock.h"

/** Gap left between one palette tile and the next, across and down. */
static constexpr float PaletteTileSpacing = 4.0f;

/** Width every palette tile is laid out at, and so the width its label has to wrap into. */
static constexpr float PaletteTileWidth = 64.0f;

/** Inset every group's content carries from the panel edge, so their backings and buttons line up down the panel. */
static constexpr float GroupOuterInset = 4.0f;

/**
 * Space every group's content carries above and below it, so the gap between one group and the next is twice this.
 * @note Both halves of that gap come from the groups themselves rather than from the slots holding them, which is what
 *       lets a rail stack groups without padding any of its slots. It is also why the gap only moves in twos — a value
 *       here is spent on both sides of every group.
 * @note Not the same number as GroupOuterInset even though the panel's gutter reads the same on all four sides, because
 *       the two axes reach that gutter of 8 differently. Across, a group can only claim 4 of it — that inset is what
 *       the toolbar buttons carry themselves, so anything else here and the groups stop lining up with each other — and
 *       the panel's border makes up the remaining 4. Down, there is nothing to line up with, so the group claims as
 *       much as the gap it wants between groups allows and the border makes up the difference. Change this and the
 *       border's vertical padding has to move against it, or the panel's top and bottom stop matching its sides; the
 *       two are commented at each other.
 */
static constexpr float GroupVerticalInset = 6.0f;

/**
 * Near-side inset carried by group content that draws straight onto the panel.
 * @note What SlimPaletteToolBar gives each of its buttons, applied by hand to the unbacked content that is not one —
 *       the check lists and the grid's leading widget. Without it they would start a few units left of the button
 *       groups, which shows as a ragged left edge down the panel now that no backing squares them up.
 */
static constexpr float UnbackedContentInset = 4.0f;

/** Gap between a group separator's label and the rule running out from it, so the two read as one line and not as one word. */
static constexpr float GroupSeparatorLabelSpacing = 6.0f;

/**
 * Work out how many lines of label a group of commands needs.
 *
 * @param Style Palette style supplying the label font and its padding.
 * @param Commands The group's commands.
 * @return One, or two if any of the labels is too wide for a tile to hold on a single line.
 * @note So a group is only ever as tall as its own labels make it. Tiles have to agree on a height or the group stops
 *       reading as a grid, but that agreement is the group's to reach — a rail of one-word labels should not be paying
 *       for the wrapped label in some other rail.
 * @note Two is the ceiling on what is reserved, not on what is shown: a label needing a third line still gets it and
 *       takes its own tile's height with it. Reserving for that case would cost every other group a line to spare one
 *       tile from standing taller than its neighbors.
 */
static int32 MeasurePaletteLabelLines(const FToolBarStyle& Style, const TArray<TSharedPtr<FUICommandInfo>>& Commands)
{
	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const float LabelWidth = PaletteTileWidth - Style.LabelPadding.GetTotalSpaceAlong<Orient_Horizontal>();

	for (const TSharedPtr<FUICommandInfo>& Command : Commands)
	{
		if (!Command.IsValid()) continue;

		if (FontMeasure->Measure(Command->GetLabel().ToString(), Style.LabelStyle.Font).X > LabelWidth)
		{
			return 2;
		}
	}

	return 1;
}

/**
 * Space and inset a group's content so it stands off the group above it.
 *
 * @param Content The group's content, backed and inset by whatever built it.
 * @return The assembled group.
 * @note A group is spacing and nothing else now that none of them is headed. Kept as its own step anyway, so the five
 *       kinds of group go on agreeing about the inset their backings and buttons line up on.
 */
static TSharedRef<SWidget> CreateGroup(const TSharedRef<SWidget>& Content)
{
	// The outer inset every kind of group shares, so their backings line up down the panel. What separates content from
	// that backing is the backing's own padding, not this.
	//
	// Even top and bottom, so the first group in the panel stands off its top edge by exactly what the last one stands
	// off the bottom by — see GroupVerticalInset for why that is not the same number as the horizontal one.
	return SNew(SBox)
		.Padding(FMargin(GroupOuterInset, GroupVerticalInset))
		[
			Content
		];
}

/**
 * Build a group of labelled buttons laid out by one of the plugin's uniform toolbar styles.
 *
 * @param CommandList The toolkit's command list, which every button resolves against.
 * @param StyleName A WorldAssemblyEd.Command* style; its NumColumns is what decides the layout.
 * @param Commands Commands to lay out.
 * @return The bare toolbar widget, backed and inset by its style but with no heading of its own.
 */
static TSharedRef<SWidget> CreateUniformToolBar(const TSharedRef<FUICommandList>& CommandList, const FName StyleName,
	const TArray<FNWorldAssemblyEdModeRail::FNRailCommand>& Commands)
{
	FSlimHorizontalUniformToolBarBuilder ToolBarBuilder(CommandList, FMultiBoxCustomization::None);
	ToolBarBuilder.SetStyle(&FNWorldAssemblyEditorStyle::Get(), StyleName);

	for (const FNWorldAssemblyEdModeRail::FNRailCommand& Entry : Commands)
	{
		if (!Entry.Command.IsValid()) continue;

		// This builder overrides only the FButtonArgs overload, which hides the plain command one the palette
		// groups use. Leaving UserInterfaceActionType unset would be read as None and silently become a Button, so
		// it is carried across from the command instead — a toggle registered here has to still render as one.
		FButtonArgs ButtonArgs;
		ButtonArgs.Command = Entry.Command;
		ButtonArgs.CommandList = CommandList;
		ButtonArgs.UserInterfaceActionType = Entry.Command->GetUserInterfaceType();

		// Left unset where the entry has no icon of its own, which is what makes the block fall back to the
		// command's registered one rather than to the generic toolbar icon.
		if (Entry.Icon.IsSet())
		{
			ButtonArgs.IconOverride = Entry.Icon;
		}

		ToolBarBuilder.AddToolBarButton(ButtonArgs);
	}

	return ToolBarBuilder.MakeWidget();
}

/**
 * Build a group of labelled buttons laid out by one of the plugin's uniform toolbar styles.
 *
 * @param CommandList The toolkit's command list, which every button resolves against.
 * @param StyleName A WorldAssemblyEd.Command* style; its NumColumns is what decides the layout.
 * @param Commands Commands to lay out.
 * @param LeadingContent Widget to sit above the buttons in the same group, or null for the buttons alone.
 * @return The assembled group.
 * @note The leading-content half is here rather than in either caller because it is the same either way — the grid and
 *       the list differ only in the style they name, and a picker sits above one exactly as it sits above the other.
 */
static TSharedRef<SWidget> CreateUniformToolBarGroup(const TSharedRef<FUICommandList>& CommandList, const FName StyleName,
	const TArray<FNWorldAssemblyEdModeRail::FNRailCommand>& Commands, const TSharedPtr<SWidget>& LeadingContent)
{
	if (!LeadingContent.IsValid())
	{
		return CreateGroup(CreateUniformToolBar(CommandList, StyleName, Commands));
	}

	// One group rather than the leading content in a group of its own above the buttons': it is what the buttons act
	// on, and two groups are separated by the gap that separates two unrelated ones.
	//
	// Inset on the leading content only. The toolbar below carries its own, and no gap between the two — the toolbar's
	// own top padding is what separates them.
	return CreateGroup(
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.Padding(UnbackedContentInset, UnbackedContentInset, UnbackedContentInset, 0.0f)
		[
			LeadingContent.ToSharedRef()
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateUniformToolBar(CommandList, StyleName, Commands)
		]);
}

TSharedRef<SWidget> FNWorldAssemblyEdModeRail::CreateCommandPalette(const TArray<TSharedPtr<FUICommandInfo>>& Commands) const
{
	const FToolBarStyle& TileStyle = FNWorldAssemblyEditorStyle::Get().GetWidgetStyle<FToolBarStyle>("WorldAssemblyEd.CommandPalette");

	// UseAllottedSize because the tiles are a fixed size and the panel they wrap into is not: the user drags it between
	// 260 and 520 wide. SWrapBox re-reads its allotted width every tick and invalidates its own layout when it changes,
	// so the column count follows the panel with nothing to prompt it — which is the whole reason the tiles are not in
	// a multibox. SUniformWrapPanel, what a toolbar would have put them in, guesses a square when it has no geometry to
	// go on and keeps the blank row that guess reserves until something else forces a relayout.
	const TSharedRef<SWrapBox> Tiles = SNew(SWrapBox)
		.UseAllottedSize(true)
		.InnerSlotPadding(FVector2D(PaletteTileSpacing, PaletteTileSpacing));

	const int32 LabelLines = MeasurePaletteLabelLines(TileStyle, Commands);

	for (const TSharedPtr<FUICommandInfo>& Command : Commands)
	{
		if (!Command.IsValid()) continue;

		Tiles->AddSlot()
			[
				SNew(SNCommandTile)
				.Command(Command)
				.CommandList(CommandList)
				.Style(&TileStyle)
				.TileWidth(PaletteTileWidth)
				.ReservedLabelLines(LabelLines)
			];
	}

	// Through CreateBackedContent, and the one command group that keeps a backing. The tiles are a block of icons rather
	// than a row of labelled buttons, so the well is what gives that block an edge to sit against — the button groups
	// read fine without one, a floating grid of icons does not.
	return CreateBackedContent(Tiles);
}

TSharedRef<SWidget> FNWorldAssemblyEdModeRail::CreateCommandGrid(const TArray<FNRailCommand>& Commands,
	const TSharedPtr<SWidget>& LeadingContent) const
{
	return CreateUniformToolBarGroup(CommandList, "WorldAssemblyEd.CommandGrid", Commands, LeadingContent);
}

TSharedRef<SWidget> FNWorldAssemblyEdModeRail::CreateCommandList(const TArray<FNRailCommand>& Commands,
	const TSharedPtr<SWidget>& LeadingContent) const
{
	return CreateUniformToolBarGroup(CommandList, "WorldAssemblyEd.CommandList", Commands, LeadingContent);
}

TSharedRef<SWidget> FNWorldAssemblyEdModeRail::CreateBackedContent(const TSharedRef<SWidget>& Content) const
{
	// The one group that still draws a backing. The command groups gave theirs up and sit on the panel, so a well here
	// now marks out the section that is not a row of commands rather than being the frame every group shares.
	return CreateGroup(
		SNew(SBorder)
		.BorderImage(FNWorldAssemblyEditorStyle::Get().GetBrush("WorldAssemblyEd.GroupBackground"))
		.Padding(FMargin(8.0f))
		[
			Content
		]);
}

TSharedRef<SWidget> FNWorldAssemblyEdModeRail::CreateGroupSeparator(const FText& Label)
{
	// The rule itself: the whole of the widget when there is no label, and the tail of it when there is.
	const TSharedRef<SWidget> Rule = SNew(SSeparator)
		.SeparatorImage(FNWorldAssemblyEditorStyle::Get().GetBrush("WorldAssemblyEd.GroupSeparator"))
		.Thickness(1.0f);

	TSharedRef<SWidget> Content = Rule;
	if (!Label.IsEmpty())
	{
		Content = SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			// Near side carries the same inset the check lists and the toolbar buttons do, so a name lines up with the
			// text below it rather than with the rule's own edge. Far side is the gap that keeps the rule reading as
			// something running out from the name rather than striking through it.
			.Padding(UnbackedContentInset, 0.0f, GroupSeparatorLabelSpacing, 0.0f)
			[
				SNew(STextBlock)
				.Text(Label)
				.TextStyle(&FNWorldAssemblyEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("WorldAssemblyEd.GroupSeparatorLabel"))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			// Centered against the text rather than aligned to the row, which is the whole of what makes the name read
			// as sitting on the line: the row is as tall as the label, and the rule is one unit through the middle of it.
			.VAlign(VAlign_Center)
			[
				Rule
			];
	}

	return SNew(SBox)
		// Inset to the group edge horizontally, so the rule spans the same width the backings and buttons do rather
		// than running out to the panel's own margin.
		//
		// Nothing vertically, because the groups either side already bring their own: GroupVerticalInset below the one
		// it follows and the same again above the one it precedes, which lands the rule centered in the gap without it
		// having to know how wide that gap is. A labelled rule stands taller than a bare one by the height of its text
		// and needs no more than that — the run it names is what the extra height belongs to.
		.Padding(FMargin(GroupOuterInset, 0.0f))
		[
			Content
		];
}

TSharedRef<SWidget> FNWorldAssemblyEdModeRail::CreateCheckList(const TArray<TSharedPtr<FUICommandInfo>>& Commands) const
{
	const TSharedRef<SVerticalBox> Column = SNew(SVerticalBox);

	const TSharedRef<FUICommandList> Commands_CommandList = CommandList;
	for (int32 Index = 0; Index < Commands.Num(); Index++)
	{
		const TSharedPtr<FUICommandInfo>& Command = Commands[Index];
		if (!Command.IsValid()) continue;

		// Captured as a ref so the bindings below do not have to re-check validity every frame.
		const TSharedRef<const FUICommandInfo> CommandRef = Command.ToSharedRef();

		// Spacing between rows only — the group's own inset is applied to the whole column below.
		Column->AddSlot()
			.AutoHeight()
			.Padding(0.0f, Index == 0 ? 0.0f : 4.0f, 0.0f, 0.0f)
			[
				SNew(SCheckBox)
				.ToolTipText(Command->GetDescription())
				// The whole of why a long label used to run off the panel rather than shortening. SCheckBox arranges its
				// content at the content's own desired width by default, so the label was handed however much room it
				// asked for and the panel's border clipped whatever did not fit — no width was ever withheld from it, so
				// neither wrapping nor an overflow policy had anything to act on. Off, the content slot fills the row,
				// which is what gives the policy below a bound to ellipsize against. The engine's own filter bar turns it
				// off for the same reason.
				.CheckBoxContentUsesAutoWidth(false)
				.IsChecked_Lambda([Commands_CommandList, CommandRef]() { return Commands_CommandList->GetCheckState(CommandRef); })
				.IsEnabled_Lambda([Commands_CommandList, CommandRef]() { return Commands_CommandList->CanExecuteAction(CommandRef); })
				// The command's own Execute already flips the underlying setting, so the new check state is discarded:
				// these are toggles, and driving them from the reported state would double-apply the change.
				.OnCheckStateChanged_Lambda([Commands_CommandList, CommandRef](ECheckBoxState) { Commands_CommandList->ExecuteAction(CommandRef); })
				[
					SNew(STextBlock)
					.Text(Command->GetLabel())
					.Margin(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
					// Shortened rather than wrapped. A setting's name belongs on the line its checkbox is on, and wrapping
					// it puts the rows out of step with each other at exactly the width the panel is hardest to read at.
					// The tooltip already carries the description, so nothing is lost to the ellipsis that was not.
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					// The checkbox label is a name, not a heading — the same dim the group's own heading uses reads
					// as secondary next to the palette buttons rather than competing with them.
					.ColorAndOpacity(FStyleColors::Foreground)
				]
			];
	}

	// Straight onto the panel like the command groups, carrying by hand the near-side inset a backing used to give it.
	return CreateGroup(
		SNew(SBox)
		.Padding(FMargin(UnbackedContentInset, UnbackedContentInset, 0.0f, 0.0f))
		[
			Column
		]);
}

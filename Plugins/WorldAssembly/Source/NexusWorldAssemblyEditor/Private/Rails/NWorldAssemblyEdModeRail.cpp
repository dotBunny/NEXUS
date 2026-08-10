// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Rails/NWorldAssemblyEdModeRail.h"

#include "NWorldAssemblyEditorStyle.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/SToolBarButtonBlock.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNCommandTile.h"
#include "Widgets/Text/STextBlock.h"

/** Gap left between one palette tile and the next, across and down. */
static constexpr float PaletteTileSpacing = 4.0f;

/** Width every palette tile is laid out at, and so the width its label has to wrap into. */
static constexpr float PaletteTileWidth = 64.0f;

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

/** @return The heading every titled group is topped with, so the four of them stay identical. */
static TSharedRef<SWidget> CreateGroupHeading(const FText& Title)
{
	return SNew(STextBlock)
		.Text(Title)
		.Font(FAppStyle::Get().GetFontStyle("EditorModesPanel.CategoryFontStyle"))
		.ColorAndOpacity(FStyleColors::White25);
}

/**
 * Stack a group's content under its heading, or on its own where it has none.
 *
 * @param Title Heading shown above the content, or empty for a group that goes without one.
 * @param Content The group's content, backed and inset by whatever built it.
 * @return The assembled group.
 * @note An empty title drops the heading rather than drawing a blank one, for a group whose content already says what
 *       a heading would. The inset the heading carried moves to the content, so an untitled group still stands off the
 *       one above it rather than sitting flush against it.
 */
static TSharedRef<SWidget> CreateGroup(const FText& Title, const TSharedRef<SWidget>& Content)
{
	const TSharedRef<SVerticalBox> Group = SNew(SVerticalBox);
	const bool bHasHeading = !Title.IsEmpty();

	if (bHasHeading)
	{
		Group->AddSlot()
			.AutoHeight()
			.Padding(8.0f, 10.0f, 8.0f, 2.0f)
			[
				CreateGroupHeading(Title)
			];
	}

	// The outer inset every kind of group shares, so their backings line up down the panel. What separates content from
	// that backing is the backing's own padding, not this.
	Group->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.Padding(4.0f, bHasHeading ? 0.0f : 10.0f, 4.0f, 6.0f)
		[
			Content
		];

	return Group;
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

	return CreateGroup(Title, ToolBarBuilder.MakeWidget());
}

TSharedRef<SWidget> FNWorldAssemblyEdModeRail::CreateTitledCommandPalette(const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands) const
{
	const FToolBarStyle& TileStyle = FNWorldAssemblyEditorStyle::Get().GetWidgetStyle<FToolBarStyle>("WorldAssemblyEd.TitledCommandPalette");

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

	// Through CreateTitledContent rather than drawing its own backing: with the tiles out of a toolbar there is no
	// style painting one for this group, and the recessed well the other groups sit in is what keeps it a peer of them.
	return CreateTitledContent(Title, Tiles);
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
	return CreateGroup(Title,
		SNew(SBorder)
		.BorderImage(FNWorldAssemblyEditorStyle::Get().GetBrush("WorldAssemblyEd.TitledGroupBackground"))
		// Matches what the toolbar styles carry as their BackgroundPadding, so content lands the same distance inside
		// its backing as a button does inside its.
		.Padding(FMargin(8.0f))
		[
			Content
		]);
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

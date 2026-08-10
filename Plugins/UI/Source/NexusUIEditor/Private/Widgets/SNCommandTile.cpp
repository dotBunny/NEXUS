// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Widgets/SNCommandTile.h"

#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "Fonts/FontMeasure.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "NexusUIEditor"

/**
 * @param InCommand The command whose description and chord the tooltip is built from.
 * @return The command's description with its keyboard shortcut appended, which is what a toolbar button would show.
 */
static FText MakeToolTipText(const TSharedRef<const FUICommandInfo>& InCommand)
{
	const FText Description = InCommand->GetDescription();

	if (!InCommand->GetFirstValidChord()->IsValidChord())
	{
		return Description;
	}

	return FText::Format(LOCTEXT("CommandTile_ToolTipWithChord", "{0} ({1})"), Description, InCommand->GetInputText());
}

void SNCommandTile::Construct(const FArguments& InArgs)
{
	if (!InArgs._Command.IsValid() || !InArgs._CommandList.IsValid())
	{
		return;
	}

	const TSharedRef<const FUICommandInfo> Command = InArgs._Command.ToSharedRef();
	const TSharedRef<const FUICommandList> CommandList = InArgs._CommandList.ToSharedRef();
	const FToolBarStyle& Style = *InArgs._Style;

	// On the tile rather than on the button inside it: a command whose visibility predicate hides it should give its
	// place in the group back to the tiles after it, and only collapsing the whole tile does that.
	SetVisibility(TAttribute<EVisibility>::CreateLambda([CommandList, Command]() { return CommandList->GetVisibility(Command); }));

	const TSharedRef<SWidget> TileContent = CreateTileContent(Command, Style, InArgs._TileWidth, InArgs._ReservedLabelLines);
	const FText ToolTipText = MakeToolTipText(Command);
	const TAttribute<bool> IsEnabled =
		TAttribute<bool>::CreateLambda([CommandList, Command]() { return CommandList->CanExecuteAction(Command); });

	// The same split SToolBarButtonBlock makes, and for the same reason: a check box is what can draw itself held down,
	// so anything with a state to report — toggles, radio buttons, checks — has to be one.
	if (Command->GetUserInterfaceType() == EUserInterfaceActionType::Button)
	{
		ChildSlot
		[
			SNew(SButton)
			.ButtonStyle(&Style.ButtonStyle)
			// Cleared exactly as a toolbar button clears it. SButton adds ContentPadding to the style's NormalPadding
			// rather than choosing between them, so leaving the default 4 by 2 here would inset the tile twice.
			.ContentPadding(FMargin(0.0f))
			.ToolTipText(ToolTipText)
			.IsEnabled(IsEnabled)
			.OnClicked_Lambda([CommandList, Command]()
			{
				CommandList->ExecuteAction(Command);
				return FReply::Handled();
			})
			[
				TileContent
			]
		];

		return;
	}

	ChildSlot
	[
		SNew(SCheckBox)
		.Style(&Style.ToggleButton)
		// No padding set, unlike the button above: a check box only falls back to its style's padding while nothing
		// overrides it, and the toggle style's is the 6 units above and below that give the tile its breathing room.
		// AutoWidth off for the same reason a toolbar turns it off — the content is sized by the tile, not by itself.
		.CheckBoxContentUsesAutoWidth(false)
		.ToolTipText(ToolTipText)
		.IsEnabled(IsEnabled)
		.IsChecked_Lambda([CommandList, Command]() { return CommandList->GetCheckState(Command); })
		// The command's own Execute already flips whatever the check state reflects, so the reported state is
		// discarded — driving the toggle from it would apply the change twice.
		.OnCheckStateChanged_Lambda([CommandList, Command](ECheckBoxState) { CommandList->ExecuteAction(Command); })
		[
			TileContent
		]
	];
}

TSharedRef<SWidget> SNCommandTile::CreateTileContent(const TSharedRef<const FUICommandInfo>& InCommand, const FToolBarStyle& InStyle,
	const float InTileWidth, const int32 InReservedLabelLines)
{
	// Measured rather than declared, so the tile is only ever as tall as the icon, the paddings and the lines it is
	// keeping room for. Height set on the label alone for the same reason: reserving it on the tile would mean knowing
	// what the icon and the two paddings above come to, and being wrong about any of them shows up as dead space.
	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const float ReservedLabelHeight = InReservedLabelLines * FontMeasure->GetMaxCharacterHeight(InStyle.LabelStyle.Font);

	return SNew(SBox)
		.WidthOverride(InTileWidth)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			// The label-visible variant, which is the one a toolbar button reads whenever it has a label to show. On
			// PaletteToolBar it is the FToolBarStyle default of zero, leaving LabelPadding as the whole of the gap
			// between icon and label — the same gap the palette buttons this replaces have.
			.Padding(InStyle.IconPaddingWithVisibleLabel)
			[
				SNew(SImage)
				.Image(InCommand->GetIcon().GetIcon())
				// So the icon picks up the button's hovered and checked foregrounds the way a toolbar button's does.
				.ColorAndOpacity(FSlateColor::UseForeground())
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			// Filled, not centered: AutoWrapText breaks against the width the text block is given, and a centered slot
			// gives it only the width it asked for — which for unwrapped text is the whole label on one line. Filling
			// hands it the tile's width to wrap into, and Justification is then what centers the lines within it.
			.HAlign(HAlign_Fill)
			.Padding(InStyle.LabelPadding)
			[
				SNew(SBox)
				.MinDesiredHeight(ReservedLabelHeight)
				[
					SNew(STextBlock)
					.Text(InCommand->GetLabel())
					.TextStyle(&InStyle.LabelStyle)
					.Justification(ETextJustify::Center)
					.AutoWrapText(true)
				]
			]
		];
}

#undef LOCTEXT_NAMESPACE

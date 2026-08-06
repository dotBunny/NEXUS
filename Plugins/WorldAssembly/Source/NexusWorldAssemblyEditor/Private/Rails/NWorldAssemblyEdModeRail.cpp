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
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

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
			ToolBarBuilder.MakeWidget()
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
					SNew(STextBlock)
					.Text(Command->GetLabel())
					.Margin(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
				]
			];
	}

	return CreateTitledContent(Title, Column);
}

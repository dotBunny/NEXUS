// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Rails/NWorldAssemblyEdModeRail.h"

#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> FNWorldAssemblyEdModeRail::CreateTitledCommandList(const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands) const
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
			SNew(STextBlock)
			.Text(Title)
			.Font(FAppStyle::Get().GetFontStyle("EditorModesPanel.CategoryFontStyle"))
			.ColorAndOpacity(FStyleColors::White25)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f, 0.0f, 4.0f, 6.0f)
		[
			ToolBarBuilder.MakeWidget()
		];
}

TSharedRef<SWidget> FNWorldAssemblyEdModeRail::CreateTitledCheckList(const FText& Title, const TArray<TSharedPtr<FUICommandInfo>>& Commands) const
{
	const TSharedRef<SVerticalBox> Column = SNew(SVerticalBox);

	Column->AddSlot()
		.AutoHeight()
		.Padding(8.0f, 10.0f, 8.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(Title)
			.Font(FAppStyle::Get().GetFontStyle("EditorModesPanel.CategoryFontStyle"))
			.ColorAndOpacity(FStyleColors::White25)
		];

	const TSharedRef<FUICommandList> Commands_CommandList = CommandList;
	for (int32 Index = 0; Index < Commands.Num(); Index++)
	{
		const TSharedPtr<FUICommandInfo>& Command = Commands[Index];
		if (!Command.IsValid()) continue;

		// The last row carries the group's closing padding, so a checklist terminates as clearly as the toolbar
		// CreateTitledCommandList builds. Without it the final checkbox runs straight into the next group's heading,
		// leaving no visual boundary between the two.
		const bool bIsLast = Index == Commands.Num() - 1;

		// Captured as a ref so the bindings below do not have to re-check validity every frame.
		const TSharedRef<const FUICommandInfo> CommandRef = Command.ToSharedRef();

		Column->AddSlot()
			.AutoHeight()
			.Padding(12.0f, 2.0f, 8.0f, bIsLast ? 8.0f : 2.0f)
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

	return Column;
}

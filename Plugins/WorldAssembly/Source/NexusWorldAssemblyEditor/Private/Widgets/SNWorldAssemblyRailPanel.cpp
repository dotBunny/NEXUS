// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Widgets/SNWorldAssemblyRailPanel.h"

#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblyEditorStyle.h"
#include "Styling/AppStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SNWorldAssemblyRailPanel::Construct(const FArguments& InArgs)
{
	State = InArgs._State;
	ActiveToolName = InArgs._ActiveToolName;

	// Clamped so an empty selection lands on a real slot rather than leaving the switcher with none. The strip only
	// reports INDEX_NONE when no category at all is available, which the World category makes impossible in practice.
	ContentSwitcher = SNew(SWidgetSwitcher)
		.WidgetIndex_Lambda([this]() { return State.IsValid() ? FMath::Max(State->GetActiveIndex(), 0) : 0; });

	if (State.IsValid())
	{
		for (const TSharedRef<FNWorldAssemblyEdModeRail>& Rail : State->GetRails())
		{
			ContentSwitcher->AddSlot()
				[
					CreateCategoryContent(Rail)
				];
		}
	}

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			ContentSwitcher.ToSharedRef()
		]

		// Outside the scroll box each category's content is wrapped in, so neither can be scrolled out of reach.
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateActiveToolRow()
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateWarningFooter()
		]
	];
}

TSharedRef<SWidget> SNWorldAssemblyRailPanel::CreateCategoryContent(const TSharedRef<FNWorldAssemblyEdModeRail>& Rail)
{
	const TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);

	if (const TSharedPtr<SWidget> Header = Rail->CreateHeader(); Header.IsValid())
	{
		Content->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(4.0f, 4.0f, 4.0f, 2.0f)
			[
				Header.ToSharedRef()
			];
	}

	if (const TSharedPtr<SWidget> RailContent = Rail->CreateContent(); RailContent.IsValid())
	{
		Content->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			[
				RailContent.ToSharedRef()
			];
	}

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			Content
		];
}

TSharedRef<SWidget> SNWorldAssemblyRailPanel::CreateActiveToolRow() const
{
	const TAttribute<FText> ToolName = ActiveToolName;

	return SNew(SBox)
		.Padding(FMargin(6.0f, 4.0f, 6.0f, 2.0f))
		.Visibility_Lambda([ToolName]()
		{
			return ToolName.Get(FText::GetEmpty()).IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
		})
		[
			SNew(SBorder)
			// The same well the titled groups sit in, rounded to match them — a square box among rounded ones reads
			// as an oversight rather than a distinction.
			.BorderImage(FNWorldAssemblyEditorStyle::Get().GetBrush("WorldAssemblyEd.TitledGroupBackground"))
			.Padding(FMargin(.0f, 4.0f))
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
					.ColorAndOpacity(FStyleColors::AccentBlue)
					.Text_Lambda([ToolName]() { return ToolName.Get(FText::GetEmpty()); })
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
					.ToolTipText(NSLOCTEXT("NexusWorldAssemblyEditor", "StopTool_ToolTip", "Stop the running tool."))
					.OnClicked_Lambda([]()
					{
						UNWorldAssemblyEdMode::EndActiveTool();
						return FReply::Handled();
					})
					[
						SNew(SImage)
						.Image(FAppStyle::Get().GetBrush("Icons.X"))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
				]
			]
		];
}

TSharedRef<SWidget> SNWorldAssemblyRailPanel::CreateWarningFooter()
{
	// Visibility goes on the outer box rather than the border: with it on the border the padding below would survive
	// the collapse and leave a gap at the bottom of the panel with nothing to warn about.
	return SNew(SBox)
		.Padding(FMargin(6.0f, 2.0f, 6.0f, 6.0f))
		.Visibility_Lambda([]()
		{
			return UNWorldAssemblyEdMode::GetWarningText().IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
		})
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::Get().GetBrush("RoundedWarning"))
			.Padding(FMargin(8.0f, 6.0f))
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				.ColorAndOpacity(FStyleColors::Warning)
				.Text_Static(&UNWorldAssemblyEdMode::GetWarningText)
			]
		];
}

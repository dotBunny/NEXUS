// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Widgets/SNWorldAssemblyRailPanel.h"

#include "EdMode/NWorldAssemblyEdMode.h"
#include "NWorldAssemblyEditorStyle.h"
#include "Styling/AppStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SNWorldAssemblyRailPanel::Construct(const FArguments& InArgs)
{
	State = InArgs._State;

	// Clamped so an empty selection lands on a real slot rather than leaving the switcher with none. Reached whenever
	// the user closes the panel from the strip, but never painted: the toolkit collapses the whole overlay on the same
	// condition, so the slot this falls back to is only ever the one a hidden switcher is pointing at.
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

		// Outside the scroll box each category's content is wrapped in, so it cannot be scrolled out of reach.
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateWarningFooter()
		]
	];
}

TSharedRef<SWidget> SNWorldAssemblyRailPanel::CreateCategoryContent(const TSharedRef<FNWorldAssemblyEdModeRail>& Rail)
{
	// Straight into the scroll box. A rail's content is already a column of groups, so stacking it inside a box of our
	// own only added a layout pass — the pickers that used to need a slot above it are part of that column now.
	const TSharedPtr<SWidget> RailContent = Rail->CreateContent();

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			RailContent.IsValid() ? RailContent.ToSharedRef() : SNullWidget::NullWidget
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

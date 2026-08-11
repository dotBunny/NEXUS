// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Widgets/SNWorldAssemblyRail.h"

#include "NWorldAssemblyEditorStyle.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

/** How often the strip re-checks which categories still have anything in the level to act on. */
static constexpr float AvailabilityPollInterval = 0.25f;

void SNWorldAssemblyRail::Construct(const FArguments& InArgs)
{
	State = InArgs._State;
	CategoryCommandList = MakeShared<FUICommandList>();

	ChildSlot
	[
		CreateCategoryToolBar()
	];

	RegisterActiveTimer(AvailabilityPollInterval, FWidgetActiveTimerDelegate::CreateSP(this, &SNWorldAssemblyRail::TickAvailability));
}

EActiveTimerReturnType SNWorldAssemblyRail::TickAvailability(double, float)
{
	if (State.IsValid())
	{
		State->RefreshAvailability();
	}

	return EActiveTimerReturnType::Continue;
}

TSharedRef<SWidget> SNWorldAssemblyRail::CreateCategoryToolBar()
{
	// Icon-only, on no background of its own — see WorldAssemblyEd.CategoryToolBar. Deliberately no SetLabelVisibility
	// call: the style's ShowLabels is what suppresses the text, and setting visibility here would override it.
	FVerticalToolBarBuilder ToolBarBuilder(CategoryCommandList, FMultiBoxCustomization::None);
	ToolBarBuilder.SetStyle(&FNWorldAssemblyEditorStyle::Get(), "WorldAssemblyEd.CategoryToolBar");

	if (!State.IsValid()) return ToolBarBuilder.MakeWidget();

	const TSharedPtr<FNWorldAssemblyRailState> RailState = State;
	const TArray<TSharedRef<FNWorldAssemblyEdModeRail>>& Rails = RailState->GetRails();

	for (int32 Index = 0; Index < Rails.Num(); Index++)
	{
		const TSharedPtr<FUICommandInfo> CategoryCommand = Rails[Index]->GetCategoryCommand();
		if (!CategoryCommand.IsValid()) continue;

		// Availability drives visibility rather than enablement: a category with nothing in the level to act on is not
		// one the user can do anything about, so a greyed button would spend a strip slot communicating a dead end.
		// SToolBarButtonBlock resolves this through the command list and collapses the block, so the buttons below
		// simply close the gap.
		// Toggle rather than select: clicking the lit button closes the panel and leaves no category selected, which is
		// what the ToggleButton these are registered as has always looked like it would do.
		CategoryCommandList->MapAction(
			CategoryCommand,
			FExecuteAction::CreateLambda([RailState, Index]() { RailState->ToggleActiveIndex(Index); }),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([RailState, Index]() { return RailState->GetActiveIndex() == Index; }),
			FIsActionButtonVisible::CreateLambda([RailState, Index]() { return RailState->IsAvailable(Index); }));

		ToolBarBuilder.AddToolBarButton(CategoryCommand);
	}

	return ToolBarBuilder.MakeWidget();
}

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "EdMode/NWorldAssemblyRailState.h"

#include "EdMode/NWorldAssemblyEdMode.h"

FNWorldAssemblyRailState::FNWorldAssemblyRailState(TArray<TSharedRef<FNWorldAssemblyEdModeRail>> InRails)
	: Rails(MoveTemp(InRails))
{
	RefreshAvailability();

	// First available category rather than the first one outright: Cell and Junction both sit above nothing when the
	// level has no cell actor, and opening on a category whose button is hidden would show content with no way back
	// to it. RefreshAvailability cannot do this itself — it only moves off a category that has become unavailable,
	// and INDEX_NONE was never available to begin with.
	for (int32 Index = 0; Index < Rails.Num(); Index++)
	{
		if (!IsAvailable(Index)) continue;

		ActiveIndex = Index;
		break;
	}
}

bool FNWorldAssemblyRailState::IsAvailable(const int32 Index) const
{
	return Available.IsValidIndex(Index) && Available[Index];
}

void FNWorldAssemblyRailState::SetActiveIndex(const int32 Index)
{
	if (ActiveIndex == Index) return;

	ActiveIndex = Index;
	UNWorldAssemblyEdMode::EndActiveTool();
}

void FNWorldAssemblyRailState::RefreshAvailability()
{
	Available.SetNum(Rails.Num());

	for (int32 Index = 0; Index < Rails.Num(); Index++)
	{
		// An unset predicate means the category is always relevant. World relies on this, which is what makes it a
		// safe fallback target below.
		const TAttribute<bool> Predicate = Rails[Index]->GetAvailable();
		Available[Index] = !Predicate.IsSet() || Predicate.Get();
	}

	if (IsAvailable(ActiveIndex)) return;

	for (int32 Index = 0; Index < Rails.Num(); Index++)
	{
		if (!IsAvailable(Index)) continue;

		SetActiveIndex(Index);
		return;
	}
}

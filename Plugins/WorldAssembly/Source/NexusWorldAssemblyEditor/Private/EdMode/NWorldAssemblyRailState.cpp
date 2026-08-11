// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "EdMode/NWorldAssemblyRailState.h"

#include "EdMode/NWorldAssemblyEdMode.h"

FNWorldAssemblyRailState::FNWorldAssemblyRailState(TArray<TSharedRef<FNWorldAssemblyEdModeRail>> InRails)
	: Rails(MoveTemp(InRails))
{
	// Leaves the selection on the first available category — its fallback fires here, because the INDEX_NONE this
	// starts on is never available. First available rather than the first outright: Cell and Junction both sit above
	// nothing when the level has no cell actor, and opening on a category whose button is hidden would show content
	// with no way back to it.
	RefreshAvailability();

	// ...which what the level actually holds then overrides.
	SeedActiveIndex();
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

void FNWorldAssemblyRailState::SeedActiveIndex()
{
	for (int32 Index = 0; Index < Rails.Num(); Index++)
	{
		if (!IsAvailable(Index) || !Rails[Index]->ShouldAutoSelect()) continue;

		// Assigned rather than set through SetActiveIndex: there is no tool running to end at construction, and the
		// mode this would ask to end one on is still being entered.
		ActiveIndex = Index;
		return;
	}
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

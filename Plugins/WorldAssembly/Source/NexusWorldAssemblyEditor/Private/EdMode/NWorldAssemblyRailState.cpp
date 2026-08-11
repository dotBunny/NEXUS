// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "EdMode/NWorldAssemblyRailState.h"

#include "EdMode/NWorldAssemblyEdMode.h"

FNWorldAssemblyRailState::FNWorldAssemblyRailState(TArray<TSharedRef<FNWorldAssemblyEdModeRail>> InRails)
	: Rails(MoveTemp(InRails))
{
	// Fills the availability cache the seed then reads. It cannot seed on its own account any more: its fallback no
	// longer fires from INDEX_NONE, which is now a state the user can ask for rather than only the state this starts in.
	RefreshAvailability();

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

void FNWorldAssemblyRailState::ToggleActiveIndex(const int32 Index)
{
	SetActiveIndex(ActiveIndex == Index ? INDEX_NONE : Index);
}

void FNWorldAssemblyRailState::SeedActiveIndex()
{
	// Assigned rather than set through SetActiveIndex throughout: there is no tool running to end at construction, and
	// the mode this would ask to end one on is still being entered.
	for (int32 Index = 0; Index < Rails.Num(); Index++)
	{
		if (!IsAvailable(Index) || !Rails[Index]->ShouldAutoSelect()) continue;

		ActiveIndex = Index;
		return;
	}

	for (int32 Index = 0; Index < Rails.Num(); Index++)
	{
		if (!IsAvailable(Index)) continue;

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

	// Closed on purpose is not something to recover from — see the header. Only a category that has gone out from
	// under the selection is.
	if (ActiveIndex == INDEX_NONE || IsAvailable(ActiveIndex)) return;

	for (int32 Index = 0; Index < Rails.Num(); Index++)
	{
		if (!IsAvailable(Index)) continue;

		SetActiveIndex(Index);
		return;
	}
}

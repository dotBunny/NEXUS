#include "Components/NComboBoxString.h"

UComboBoxString::FOnSelectionChangedEvent UNComboBoxString::EmptySelectionChanged;

void UNComboBoxString::SetSelectedOption_NoBroadcast(const FString Option)
{
	if (!GetSelectedOption().Equals(Option))
	{
		const bool bPreviousShouldBroadcastState = bShouldBroadcastState;
		bShouldBroadcastState = false;
		CachedOnSelectionChanged = OnSelectionChanged;
		OnSelectionChanged = EmptySelectionChanged;

		SetSelectedOption(Option);
		
		OnSelectionChanged = CachedOnSelectionChanged;
		bShouldBroadcastState = bPreviousShouldBroadcastState;
	}
}

void UNComboBoxString::SetSelectedIndex_NoBroadcast(const int32 Index)
{
	if (GetSelectedIndex() != Index)
	{
		const bool bPreviousShouldBroadcastState = bShouldBroadcastState;
		bShouldBroadcastState = false;
		CachedOnSelectionChanged = OnSelectionChanged;
		OnSelectionChanged = EmptySelectionChanged;
		
		SetSelectedIndex(Index);

		OnSelectionChanged = CachedOnSelectionChanged;
		bShouldBroadcastState = bPreviousShouldBroadcastState;
	}
}

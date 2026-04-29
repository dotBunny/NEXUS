#include "Components/NComboBoxString.h"

UComboBoxString::FOnSelectionChangedEvent UNComboBoxString::EmptyChangedEvent;

void UNComboBoxString::SetSelectedOption_NoBroadcast(const FString Option)
{
	if (!GetSelectedOption().Equals(Option))
	{
		const bool bPreviousShouldBroadcastState = bShouldBroadcastState;
		bShouldBroadcastState = false;
		CachedChangedEvent = OnSelectionChanged;
		OnSelectionChanged = EmptyChangedEvent;

		SetSelectedOption(Option);
		
		OnSelectionChanged = CachedChangedEvent;
		bShouldBroadcastState = bPreviousShouldBroadcastState;
	}
}

void UNComboBoxString::SetSelectedIndex_NoBroadcast(const int32 Index)
{
	if (GetSelectedIndex() != Index)
	{
		const bool bPreviousShouldBroadcastState = bShouldBroadcastState;
		bShouldBroadcastState = false;
		CachedChangedEvent = OnSelectionChanged;
		OnSelectionChanged = EmptyChangedEvent;
		
		SetSelectedIndex(Index);

		OnSelectionChanged = CachedChangedEvent;
		bShouldBroadcastState = bPreviousShouldBroadcastState;
	}
}

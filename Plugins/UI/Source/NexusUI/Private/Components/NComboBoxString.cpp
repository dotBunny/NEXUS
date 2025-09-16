#include "Components/NComboBoxString.h"

UComboBoxString::FOnSelectionChangedEvent UNComboBoxString::EmptySelectionChanged;

void UNComboBoxString::SetSelectedOption_NoBroadcast(const FString Option)
{
	if (!GetSelectedOption().Equals(Option))
	{
		CachedOnSelectionChanged = OnSelectionChanged;
		OnSelectionChanged = EmptySelectionChanged;
		SetSelectedOption(Option);
		OnSelectionChanged = CachedOnSelectionChanged;
	}
}

void UNComboBoxString::SetSelectedIndex_NoBroadcast(const int32 Index)
{
	if (GetSelectedIndex() != Index)
	{
		CachedOnSelectionChanged = OnSelectionChanged;
		OnSelectionChanged = EmptySelectionChanged;
		SetSelectedIndex(Index);
		OnSelectionChanged = CachedOnSelectionChanged;
	}
}

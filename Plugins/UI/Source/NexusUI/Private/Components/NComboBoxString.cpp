// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Components/NComboBoxString.h"

UComboBoxString::FOnSelectionChangedEvent UNComboBoxString::EmptyChangedEvent;

void UNComboBoxString::SetSelectedOption_NoBroadcast(const FString Option)
{
	if (!GetSelectedOption().Equals(Option))
	{
		const bool bPreviousShouldBroadcastState = bShouldBroadcastState;
		bShouldBroadcastState = false;
		TGuardValue<UComboBoxString::FOnSelectionChangedEvent> EventGuard(OnSelectionChanged, EmptyChangedEvent);

		SetSelectedOption(Option);

		bShouldBroadcastState = bPreviousShouldBroadcastState;
	}
}

void UNComboBoxString::SetSelectedIndex_NoBroadcast(const int32 Index)
{
	if (GetSelectedIndex() != Index)
	{
		const bool bPreviousShouldBroadcastState = bShouldBroadcastState;
		bShouldBroadcastState = false;
		TGuardValue<UComboBoxString::FOnSelectionChangedEvent> EventGuard(OnSelectionChanged, EmptyChangedEvent);

		SetSelectedIndex(Index);

		bShouldBroadcastState = bPreviousShouldBroadcastState;
	}
}

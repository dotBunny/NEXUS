// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Components/NCheckBox.h"

FOnCheckBoxComponentStateChanged UNCheckBox::EmptyChangedDelegate;

void UNCheckBox::SetIsChecked_NoBroadcast(const bool bNewValue)
{
	
	if (IsChecked() != bNewValue)
	{
		const bool bPreviousShouldBroadcastState = bShouldBroadcastState;
		bShouldBroadcastState = false;
		CachedChangedDelegate = OnCheckStateChanged;
		OnCheckStateChanged = EmptyChangedDelegate;

		SetIsChecked(bNewValue);

		OnCheckStateChanged = CachedChangedDelegate;
		bShouldBroadcastState = bPreviousShouldBroadcastState;
	}
}

void UNCheckBox::SetCheckedState_NoBroadcast(const ECheckBoxState NewState)
{
	if (GetCheckedState() != NewState)
	{
		const bool bPreviousShouldBroadcastState = bShouldBroadcastState;
		bShouldBroadcastState = false;
		CachedChangedDelegate = OnCheckStateChanged;
		OnCheckStateChanged = EmptyChangedDelegate;

		SetCheckedState(NewState);

		OnCheckStateChanged = CachedChangedDelegate;
		bShouldBroadcastState = bPreviousShouldBroadcastState;
	}
}

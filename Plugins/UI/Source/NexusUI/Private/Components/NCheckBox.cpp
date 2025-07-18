﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Components/NCheckBox.h"

FOnCheckBoxComponentStateChanged UNCheckBox::EmptyStateChanged;

void UNCheckBox::SetIsChecked_NoBroadcast(const bool bNewValue)
{
	if (IsChecked() != bNewValue)
	{
		CachedOnStateChanged = OnCheckStateChanged;
		OnCheckStateChanged = EmptyStateChanged;
		SetIsChecked(bNewValue);
		OnCheckStateChanged = CachedOnStateChanged;
	}
}

void UNCheckBox::SetCheckedState_NoBroadcast(const ECheckBoxState NewState)
{
	if (GetCheckedState() != NewState)
	{
		CachedOnStateChanged = OnCheckStateChanged;
		OnCheckStateChanged = EmptyStateChanged;
		SetCheckedState(NewState);
		OnCheckStateChanged = CachedOnStateChanged;
	}
}

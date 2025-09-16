// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Components/NSpinBox.h"

USpinBox::FOnSpinBoxValueChangedEvent UNSpinBox::EmptyValueChanged;
USpinBox::FOnSpinBoxValueCommittedEvent UNSpinBox::EmptyValueCommitted;

void UNSpinBox::SetValue_NoBroadcast(const float NewValue)
{
	if (GetValue() != NewValue)
	{
		const bool bPreviousShouldBroadcastState = bShouldBroadcastState;
		bShouldBroadcastState = false;
		CachedOnValueChanged = OnValueChanged;
		CachedOnValueCommitted = OnValueCommitted;		
		OnValueChanged = EmptyValueChanged;
		OnValueCommitted = EmptyValueCommitted;
		
		SetValue(NewValue);
		
		OnValueChanged = CachedOnValueChanged;
		OnValueCommitted = CachedOnValueCommitted;
		bShouldBroadcastState = bPreviousShouldBroadcastState;
	}
}
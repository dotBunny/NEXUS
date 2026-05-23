// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Components/NSpinBox.h"

USpinBox::FOnSpinBoxValueChangedEvent UNSpinBox::EmptyChangedEvent;
USpinBox::FOnSpinBoxValueCommittedEvent UNSpinBox::EmptyCommittedEvent;

void UNSpinBox::SetValue_NoBroadcast(const float NewValue)
{
	if (GetValue() != NewValue)
	{
		const bool bPreviousShouldBroadcastState = bShouldBroadcastState;
		bShouldBroadcastState = false;
		TGuardValue<USpinBox::FOnSpinBoxValueChangedEvent> ChangedGuard(OnValueChanged, EmptyChangedEvent);
		TGuardValue<USpinBox::FOnSpinBoxValueCommittedEvent> CommittedGuard(OnValueCommitted, EmptyCommittedEvent);

		SetValue(NewValue);

		bShouldBroadcastState = bPreviousShouldBroadcastState;
	}
}
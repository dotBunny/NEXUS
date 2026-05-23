// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Components/NSlider.h"

FOnFloatValueChangedEvent UNSlider::EmptyChangedEvent;

void UNSlider::SetValue_NoBroadcast(const float NewValue)
{
	if (GetValue() != NewValue)
	{
		const bool bPreviousShouldBroadcastState = bShouldBroadcastState;
		bShouldBroadcastState = false;
		TGuardValue<FOnFloatValueChangedEvent> EventGuard(OnValueChanged, EmptyChangedEvent);

		SetValue(NewValue);

		bShouldBroadcastState = bPreviousShouldBroadcastState;
	}
}
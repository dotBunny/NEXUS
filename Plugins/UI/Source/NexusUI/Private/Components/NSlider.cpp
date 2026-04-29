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
		CachedChangedEvent = OnValueChanged;
		OnValueChanged = EmptyChangedEvent;
		
		SetValue(NewValue);

		OnValueChanged = CachedChangedEvent;
		bShouldBroadcastState = bPreviousShouldBroadcastState;
	}
}
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Components/NSlider.h"

FOnFloatValueChangedEvent UNSlider::EmptyValueChanged;

void UNSlider::SetValue_NoBroadcast(const float NewValue)
{
	if (GetValue() != NewValue)
	{
		CachedOnValueChanged = OnValueChanged;
		OnValueChanged = EmptyValueChanged;
		SetValue(NewValue);
		OnValueChanged = CachedOnValueChanged;
	}
}
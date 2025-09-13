// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Components/Slider.h"
#include "NSlider.generated.h"

/**
 * A wrapper around the USlider class to allow for setting the value without broadcasting events.
 */
UCLASS()
class NEXUSUI_API UNSlider final : public USlider
{
	GENERATED_BODY()
	
public:
	/**
	 * Set the value of the USlider without triggering exposed event bindings.
	 */
	UFUNCTION(BlueprintCallable, Category="NEXUS|User Interface", DisplayName="Set Value (No Broadcast)")
	void SetValue_NoBroadcast(const float NewValue);

private:
	/**
	 * Cached value of the OnValueChanged used to prevent it from being called when setting the value.
	 */
	FOnFloatValueChangedEvent CachedOnValueChanged;

	/**
	 * Empty OnValueChanged event used to swap in for the CachedOnValueChanged to prevent it from being called.
	 */
	static FOnFloatValueChangedEvent EmptyValueChanged;
};

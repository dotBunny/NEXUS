// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Components/SpinBox.h"
#include "NSpinBox.generated.h"

/**
 * A wrapper around the <code>USpinBox</code> class to allow for setting the value without broadcasting events.
 */
UCLASS()
class NEXUSUI_API UNSpinBox final : public USpinBox
{
	GENERATED_BODY()
	
public:
	/**
	 * Set the value of the <code>USpinBox</code> without triggering exposed event bindings.
	 */
	UFUNCTION(BlueprintCallable, Category="NEXUS|User Interface", DisplayName="Set Value (No Broadcast)")
	void SetValue_NoBroadcast(const float NewValue);
	
private:
	/**
	 * Cached value of the <code>OnValueChanged</code> used to prevent it from being called when setting the value.
	 */
	FOnSpinBoxValueChangedEvent CachedOnValueChanged;
	FOnSpinBoxValueCommittedEvent CachedOnValueCommitted;


	/**
	 * Empty <code>OnValueChanged</code> event used to swap in for the <code>CachedOnValueChanged</code> to prevent it from being called.
	 */
	static FOnSpinBoxValueChangedEvent EmptyValueChanged;
	static FOnSpinBoxValueCommittedEvent EmptyValueCommitted;
};

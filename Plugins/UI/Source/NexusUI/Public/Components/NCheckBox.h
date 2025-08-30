// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Components/CheckBox.h"
#include "NCheckBox.generated.h"

/**
 * A wrapper around the <code>UNCheckBox</code> class to allow for setting the checked state without broadcasting events.
 */
UCLASS()
class NEXUSUI_API UNCheckBox final : public UCheckBox
{
	GENERATED_BODY()
	
public:
	/**
	 * Set the checked state of the <code>UCheckBox</code> without triggering exposed event bindings.	 
	 */
	UFUNCTION(BlueprintCallable, Category="NEXUS|User Interface", DisplayName="Set Is Checked (No Broadcast)")
	void SetIsChecked_NoBroadcast(const bool bNewValue);

	/**
	 * Set the checked state of the <code>UCheckBox</code> without triggering exposed event bindings.	 
	 */
	UFUNCTION(BlueprintCallable, Category="NEXUS|User Interface", DisplayName="Set Chcked State (No Broadcast)")
	void SetCheckedState_NoBroadcast(const ECheckBoxState NewState);
	
private:
	/**
	 * Cached value of the <code>OnStateChanged</code> used to prevent it from being called when setting the value.
	 */
	FOnCheckBoxComponentStateChanged CachedOnStateChanged;

	/**
	 * Empty <code>OnStateChanged</code> event used to swap in for the <code>CachedOnStateChanged</code> to prevent it from being called.
	 */
	static FOnCheckBoxComponentStateChanged EmptyStateChanged;
};

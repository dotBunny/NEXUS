// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NUINamespace.h"
#include "Components/SpinBox.h"
#include "NSpinBox.generated.h"

/**
 * A wrapper around the USpinBox class to allow for setting the value without broadcasting events.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/types/spin-box/">UNSpinBox</a>
 */
UCLASS(DisplayName = "NEXUS: SpinBox", ClassGroup = UI, meta = (Category = "NEXUS"))
class NEXUSUI_API UNSpinBox : public USpinBox
{
	GENERATED_BODY()
	
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override {  return NEXUS::UI::Editor::PaletteCategory; }
#endif // WITH_EDITOR	

public:
	/**
	 * Set the value of the USpinBox without triggering exposed event bindings.
	 * @param NewValue The new value.
	 */
	UFUNCTION(BlueprintCallable, Category="NEXUS|User Interface", DisplayName="Set Value (No Broadcast)",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/spin-box/#set-value-no-broadcast"))
	void SetValue_NoBroadcast(const float NewValue);
	
private:
	/**
	 * Cached value of the OnValueChanged used to prevent it from being called when setting the value.
	 */
	FOnSpinBoxValueChangedEvent CachedOnValueChanged;
	FOnSpinBoxValueCommittedEvent CachedOnValueCommitted;


	/**
	 * Empty OnValueChanged event used to swap in for the CachedOnValueChanged to prevent it from being called.
	 */
	static FOnSpinBoxValueChangedEvent EmptyValueChanged;
	static FOnSpinBoxValueCommittedEvent EmptyValueCommitted;
};

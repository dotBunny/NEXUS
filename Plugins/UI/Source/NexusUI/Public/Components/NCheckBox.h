// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NUIMinimal.h"
#include "Components/CheckBox.h"
#include "NCheckBox.generated.h"

/**
 * A wrapper around the UNCheckBox class to allow for setting the checked state without broadcasting events.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/types/check-box/">UNCheckBox</a>
 */
UCLASS(DisplayName = "CheckBox", ClassGroup = UI, meta = (Category = "NEXUS"))
class NEXUSUI_API UNCheckBox : public UCheckBox
{
	GENERATED_BODY()
	
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override {  return NEXUS::UI::Editor::PaletteCategory; }
#endif // WITH_EDITOR
	
public:
	/**
	 * Sets if the UCheckBox is checked without triggering exposed event bindings.
	 * @param bNewValue The new value.
	 */
	UFUNCTION(BlueprintCallable, Category="NEXUS|User Interface", DisplayName="Set IsChecked (No Broadcast)",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/check-box/#set-ischecked-no-broadcast"))
	void SetIsChecked_NoBroadcast(const bool bNewValue);

	/**
	 * Set the checked state of the UCheckBox without triggering exposed event bindings.	
	 * @param NewState The new value.
	 */
	UFUNCTION(BlueprintCallable, Category="NEXUS|User Interface", DisplayName="Set Checked State (No Broadcast)",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/check-box/#set-checkedstate-no-broadcast"))
	void SetCheckedState_NoBroadcast(const ECheckBoxState NewState);
	
private:
	/**
	 * Cached value of the OnStateChanged used to prevent it from being called when setting the value.
	 */
	FOnCheckBoxComponentStateChanged CachedOnStateChanged;

	/**
	 * Empty OnStateChanged event used to swap in for the CachedOnStateChanged to prevent it from being called.
	 */
	static FOnCheckBoxComponentStateChanged EmptyStateChanged;
};

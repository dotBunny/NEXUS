// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NUINamespace.h"
#include "Components/ComboBoxString.h"
#include "NComboBoxString.generated.h"

/**
 * A wrapper around the UComboBoxString class to allow for setting the selected option without broadcasting events.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/types/combobox-string/">UNComboBoxString</a>
 */
UCLASS(DisplayName = "NEXUS: ComboBox (String)", ClassGroup = UI, meta = (Category = "NEXUS"))
class NEXUSUI_API UNComboBoxString : public UComboBoxString
{
	GENERATED_BODY()
	
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override {  return NEXUS::UI::Editor::PaletteCategory; }
#endif // WITH_EDITOR
	
public:
	/**
	 * Sets the selected option of the UComboBoxString without triggering exposed event bindings.
	 * @param Option The new option's text.
	 */
	UFUNCTION(BlueprintCallable, Category="NEXUS|User Interface", DisplayName="Set Selected Option (No Broadcast)",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/combobox-string/#set-selected-option-no-broadcast"))
	void SetSelectedOption_NoBroadcast(FString Option);

	/**
	 * Sets the selected option by index of the UComboBoxString without triggering exposed event bindings.	 
	 * @param Index The new option's index.
	 */
	UFUNCTION(BlueprintCallable, Category="NEXUS|User Interface", DisplayName="Set Selected Index (No Broadcast)",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/combobox-string/#set-selected-index-no-broadcast"))
	void SetSelectedIndex_NoBroadcast(const int32 Index);

private:
	/**
	 * Cached value of the OnSelectionChanged used to prevent it from being called when setting the value.
	 */
	FOnSelectionChangedEvent CachedOnSelectionChanged;

	/**
	 * Empty OnSelectionChanged event used to swap in for the CachedOnSelectionChanged to prevent it from being called.
	 */
	static FOnSelectionChangedEvent EmptySelectionChanged;
};

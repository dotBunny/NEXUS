﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Components/ComboBoxString.h"
#include "NComboBoxString.generated.h"

/**
 * A wrapper around the <code>UNCheckBox</code> class to allow for setting the checked state without broadcasting events.
 */
UCLASS()
class NEXUSUI_API UNComboBoxString final : public UComboBoxString
{
	GENERATED_BODY()
	
public:

	/**
	 * Sets the selected option of the <code>UComboBoxString</code> without triggering exposed event bindings.	 
	 */
	UFUNCTION(BlueprintCallable, Category="Widget")
	void SetSelectedOption_NoBroadcast(FString Option);

	/**
	 * Sets the selected option by index of the <code>UComboBoxString</code> without triggering exposed event bindings.	 
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget")
	void SetSelectedIndex_NoBroadcast(const int32 Index);

private:
	/**
	 * Cached value of the <code>OnSelectionChanged</code> used to prevent it from being called when setting the value.
	 */
	FOnSelectionChangedEvent CachedOnSelectionChanged;

	/**
	 * Empty <code>OnSelectionChanged</code> event used to swap in for the <code>CachedOnSelectionChanged</code> to prevent it from being called.
	 */
	static FOnSelectionChangedEvent EmptySelectionChanged;
};

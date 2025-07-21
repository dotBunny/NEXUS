// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "CommonNumericTextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "NSamplesUserWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNSamplesUserWidgetEventCountChanged, int, NewCount);

/**
 * A base widget used to test widgets throughout the Nexus Samples
 */
UCLASS(Config = Game)
class NEXUSSHAREDSAMPLES_API UNSamplesUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable)
	void ResetEventCount() { EventCount = 0;};

	UFUNCTION(BlueprintCallable)
	void PrepareForTest();

	UFUNCTION(BlueprintCallable)
	int IncrementEventCount()
	{
		++EventCount;
		OnEventCountChanged.Broadcast(EventCount);
		EventCounter->SetCurrentValue(EventCount);
		return EventCount;
	};

	UFUNCTION(BlueprintCallable)
	int GetEventCount() const { return EventCount; };

	UFUNCTION(BlueprintCallable, meta=(ExpandBoolAsExecs="ReturnValue"))
	// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
	bool HasEvents() { return EventCount != 0; };
	
	UFUNCTION(BlueprintCallable, meta=(ExpandBoolAsExecs="ReturnValue"))
	// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
	bool HasNoEvents() { return EventCount == 0; };
	
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FNSamplesUserWidgetEventCountChanged OnEventCountChanged;
	
	
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "NEXUS", meta=(BindWidget))
	TObjectPtr<UWidget> MonitoredWidget;

	UPROPERTY(BlueprintReadOnly, Category = "NEXUS", meta=(BindWidget))
	TObjectPtr<UCommonNumericTextBlock> EventCounter;

	UPROPERTY(BlueprintReadOnly, Category = "NEXUS", meta=(BindWidget))
	TObjectPtr<UTextBlock> CurrentValue;

private:

	UFUNCTION()
	void OnBoolDelegate(bool bNewValue);
	UFUNCTION()
	void OnFloatDelegate(float NewValue);	
	UFUNCTION()
	void OnSelectionDelegate(FString NewSelection, ESelectInfo::Type NewSelectInfo);
	UFUNCTION()
	void OnFloatCommited(float NewValue, ETextCommit::Type CommitType);

	
	void BindCheckbox();
	void BindComboBoxString();
	void BindSlider();
	void BindSpinBox();

	void UpdateCurrentValue();
	void UpdateCurrentValueFromCheckbox();
	void UpdateCurrentValueFromComboBoxString();
	void UpdateCurrentValueFromSlider();
	void UpdateCurrentValueFromSpinBox();
	
	
	int EventCount = 0;

	bool bCachedBooleanValue;
	float CachedFloatValue;
	FString CachedStringValue;
};
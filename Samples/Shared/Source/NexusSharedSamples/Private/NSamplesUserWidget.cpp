// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSamplesUserWidget.h"
#include "NCoreMinimal.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/SpinBox.h"

void UNSamplesUserWidget::PrepareForTest()
{
	if (MonitoredWidget->IsA<UCheckBox>())
	{
		Cast<UCheckBox>(MonitoredWidget)->SetIsChecked(false);
		
	}
	else if (MonitoredWidget->IsA<USlider>())
	{
		Cast<USlider>(MonitoredWidget)->SetValue(1);
	}
	else if (MonitoredWidget->IsA<USpinBox>())
	{
		Cast<USpinBox>(MonitoredWidget)->SetValue(1);
	}
	else if (MonitoredWidget->IsA<UComboBoxString>())
	{
		Cast<UComboBoxString>(MonitoredWidget)->SetSelectedIndex(0);
	}
	ResetEventCount();
}

void UNSamplesUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (MonitoredWidget == nullptr) return;
	
	if (MonitoredWidget->IsA<UCheckBox>())
	{
		BindCheckbox();
	}
	else if (MonitoredWidget->IsA<USlider>())
	{
		BindSlider();
	}
	else if (MonitoredWidget->IsA<USpinBox>())
	{
		BindSpinBox();
	}
	else if (MonitoredWidget->IsA<UComboBoxString>())
	{
		BindComboBoxString();
	}
	else
	{
		UE_LOG(LogNexusCore, Warning, TEXT("Unable to bind to unsupported widget type(%s) to UNSamplesUserWidget; clearing reference to avoid further messages."), *MonitoredWidget->GetName())
		MonitoredWidget = nullptr;
	}
}

void UNSamplesUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateCurrentValue();
}

void UNSamplesUserWidget::UpdateCurrentValue()
{
	if (MonitoredWidget == nullptr) return;
	
	if (MonitoredWidget->IsA<UCheckBox>())
	{
		UpdateCurrentValueFromCheckbox();
	}
	else if (MonitoredWidget->IsA<USlider>())
	{
		UpdateCurrentValueFromSlider();
	}
	else if (MonitoredWidget->IsA<USpinBox>())
	{
		UpdateCurrentValueFromSpinBox();
	}
	else if (MonitoredWidget->IsA<UComboBoxString>())
	{
		UpdateCurrentValueFromComboBoxString();
	}
	else
	{
		UE_LOG(LogNexusCore, Warning, TEXT("Unable to monitor widget via UNSamplesUserWidget due to an unsupported type(%s); clearing reference to avoid further messages."), *MonitoredWidget->GetName())
		MonitoredWidget = nullptr;
	}
}

void UNSamplesUserWidget::OnBoolDelegate([[maybe_unused]] bool bNewValue)
{
	IncrementEventCount();
}

void UNSamplesUserWidget::OnFloatDelegate([[maybe_unused]] float NewValue)
{
	IncrementEventCount();
}

void UNSamplesUserWidget::OnSelectionDelegate([[maybe_unused]] FString NewSelection, [[maybe_unused]] ESelectInfo::Type NewSelectInfo)
{
	IncrementEventCount();
}

void UNSamplesUserWidget::OnFloatCommited([[maybe_unused]] float NewValue, [[maybe_unused]] ETextCommit::Type CommitType)
{
	IncrementEventCount();
}

void UNSamplesUserWidget::BindCheckbox()
{
	UCheckBox* Widget = Cast<UCheckBox>(MonitoredWidget);
	if (Widget != nullptr)
	{
		Widget->OnCheckStateChanged.AddDynamic(this, &UNSamplesUserWidget::OnBoolDelegate);
		bCachedBooleanValue = Widget->IsChecked();
	}
}

void UNSamplesUserWidget::BindComboBoxString()
{
	UComboBoxString* Widget = Cast<UComboBoxString>(MonitoredWidget);
	if (Widget != nullptr)
	{
		Widget->OnSelectionChanged.AddDynamic(this, &UNSamplesUserWidget::OnSelectionDelegate);
		CachedStringValue = Widget->GetSelectedOption();
	}
}

void UNSamplesUserWidget::BindSlider()
{
	USlider* Widget = Cast<USlider>(MonitoredWidget);
	if (Widget != nullptr)
	{
		Widget->OnValueChanged.AddDynamic(this, &UNSamplesUserWidget::OnFloatDelegate);
		CachedFloatValue = Widget->GetValue();
	}
}

void UNSamplesUserWidget::BindSpinBox()
{
	USpinBox* Widget = Cast<USpinBox>(MonitoredWidget);
	if (Widget != nullptr)
	{
		Widget->OnValueChanged.AddDynamic(this, &UNSamplesUserWidget::OnFloatDelegate);
		Widget->OnValueCommitted.AddDynamic(this, &UNSamplesUserWidget::OnFloatCommited);
		CachedFloatValue = Widget->GetValue();
	}
}

void UNSamplesUserWidget::UpdateCurrentValueFromCheckbox()
{
	const UCheckBox* Widget = Cast<UCheckBox>(MonitoredWidget);
	if (Widget != nullptr)
	{
		const bool bCheck = Widget->IsChecked();
		if (bCheck != bCachedBooleanValue)
		{
			if (bCheck)
			{
				CurrentValue->SetText(FText::FromString("Checked"));
			}
			else
			{
				CurrentValue->SetText(FText::FromString("Unchecked"));
			}
			bCachedBooleanValue = bCheck;
		}
	}
}

void UNSamplesUserWidget::UpdateCurrentValueFromComboBoxString()
{
	const UComboBoxString* Widget = Cast<UComboBoxString>(MonitoredWidget);
	if (Widget != nullptr)
	{
		const FString Check = Widget->GetSelectedOption();
		if (Check != CachedStringValue)
		{
			CurrentValue->SetText(FText::FromString(Check));
			CachedStringValue = Check;
		}
		
	}
}

void UNSamplesUserWidget::UpdateCurrentValueFromSlider()
{
	USlider* Widget = Cast<USlider>(MonitoredWidget);
	if (Widget != nullptr)
	{
		const float Check = Widget->GetValue();
		if (Check != CachedFloatValue)
		{
			CurrentValue->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Check)));
			CachedFloatValue = Check;
		}
	}
}

void UNSamplesUserWidget::UpdateCurrentValueFromSpinBox()
{
	USpinBox* Widget = Cast<USpinBox>(MonitoredWidget);
	if (Widget != nullptr)
	{
		const float Check = Widget->GetValue();
		if (Check != CachedFloatValue)
		{
			CurrentValue->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Check)));
			CachedFloatValue = Check;
		}
	}
}

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
		N_LOG_WARNING("[UNSamplesUserWidget::NativeConstruct] Unsupported widget type: %s", *MonitoredWidget->GetName())
	}
}

void UNSamplesUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateCurrentValue();
}

void UNSamplesUserWidget::UpdateCurrentValue()
{
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
		N_LOG_WARNING("[UNSamplesUserWidget::UpdateCurrentValue] Unsupported widget type: %s", *MonitoredWidget->GetName())
	}
}

void UNSamplesUserWidget::OnBoolDelegate(bool bNewValue)
{
	IncrementEventCount();
}

void UNSamplesUserWidget::OnFloatDelegate(float NewValue)
{
	IncrementEventCount();
}

void UNSamplesUserWidget::OnSelectionDelegate(FString NewSelection, ESelectInfo::Type NewSelectInfo)
{
	IncrementEventCount();
}

void UNSamplesUserWidget::OnFloatCommited(float NewValue, ETextCommit::Type CommitType)
{
	IncrementEventCount();
}

void UNSamplesUserWidget::BindCheckbox()
{
	if (UCheckBox* Widget = Cast<UCheckBox>(MonitoredWidget))
	{
		Widget->OnCheckStateChanged.AddDynamic(this, &UNSamplesUserWidget::OnBoolDelegate);
		bCachedBooleanValue = Widget->IsChecked();
	}
}

void UNSamplesUserWidget::BindComboBoxString()
{
	if (UComboBoxString* Widget = Cast<UComboBoxString>(MonitoredWidget))
	{
		Widget->OnSelectionChanged.AddDynamic(this, &UNSamplesUserWidget::OnSelectionDelegate);
		CachedStringValue = Widget->GetSelectedOption();
	}
}

void UNSamplesUserWidget::BindSlider()
{
	if (USlider* Widget = Cast<USlider>(MonitoredWidget))
	{
		Widget->OnValueChanged.AddDynamic(this, &UNSamplesUserWidget::OnFloatDelegate);
		CachedFloatValue = Widget->GetValue();
	}
}

void UNSamplesUserWidget::BindSpinBox()
{
	if (USpinBox* Widget = Cast<USpinBox>(MonitoredWidget))
	{
		Widget->OnValueChanged.AddDynamic(this, &UNSamplesUserWidget::OnFloatDelegate);
		Widget->OnValueCommitted.AddDynamic(this, &UNSamplesUserWidget::OnFloatCommited);
		CachedFloatValue = Widget->GetValue();
	}
}

void UNSamplesUserWidget::UpdateCurrentValueFromCheckbox()
{
	if (const UCheckBox* Widget = Cast<UCheckBox>(MonitoredWidget))
	{
		if (const bool bCheck = Widget->IsChecked();
			bCheck != bCachedBooleanValue)
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
	if (const UComboBoxString* Widget = Cast<UComboBoxString>(MonitoredWidget))
	{
		if (const FString Check = Widget->GetSelectedOption();
			Check != CachedStringValue)
		{
			CurrentValue->SetText(FText::FromString(Check));
			CachedStringValue = Check;
		}
		
	}
}

void UNSamplesUserWidget::UpdateCurrentValueFromSlider()
{
	if (USlider* Widget = Cast<USlider>(MonitoredWidget))
	{
		if (const float Check = Widget->GetValue();
			Check != CachedFloatValue)
		{
			CurrentValue->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Check)));
			CachedFloatValue = Check;
		}
	}
}

void UNSamplesUserWidget::UpdateCurrentValueFromSpinBox()
{
	if (USpinBox* Widget = Cast<USpinBox>(MonitoredWidget))
	{
		if (const float Check = Widget->GetValue();
			Check != CachedFloatValue)
		{
			CurrentValue->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Check)));
			CachedFloatValue = Check;
		}
	}
}

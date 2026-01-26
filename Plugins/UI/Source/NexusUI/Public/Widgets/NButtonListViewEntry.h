// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonTextBlock.h"
#include "INListViewEntry.h"
#include "NColor.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "NButtonListViewEntry.generated.h"

UCLASS(BlueprintType)
class NEXUSUI_API UNButtonListViewEntryObject : public UObject
{
	GENERATED_BODY()

	DECLARE_DELEGATE( OnButtonPressedDelegate );
public:
	
	UFUNCTION(BlueprintCallable, Category = "Text")
	FText GetText() const { return Text; }
	
	UFUNCTION(BlueprintCallable, Category = "Text")
	void SetText(const FText& InText) { Text = InText; }

	UFUNCTION()
	// ReSharper disable once CppMemberFunctionMayBeConst
	void OnPressed() { OnPressedEvent.ExecuteIfBound(); };
	
	OnButtonPressedDelegate OnPressedEvent;
	
	ENColor HoverStateForegroundColor = ENColor::NC_White;
	ENColor HoverStateBackgroundColor = ENColor::NC_BlueLight;
	
	ENColor PressedStateForegroundColor = ENColor::NC_White;
	ENColor PressedStateBackgroundColor = ENColor::NC_BlueDark;
	
	ENColor UnhoveredStateForegroundColor = ENColor::NC_White;
	ENColor UnhoveredStateBackgroundColor = ENColor::NC_GreyDark;
	
private:
	FText Text;
};


UCLASS(ClassGroup = "NEXUS", DisplayName = "Button ListView Entry", BlueprintType, Blueprintable, HideDropdown)
class NEXUSUI_API UNButtonListViewEntry : public UUserWidget, public INListViewEntry
{
	GENERATED_BODY()

	virtual void NativeConstruct() override
	{
		Super::NativeConstruct();
		
		if (Button != nullptr)
		{
			Button->OnClicked.AddDynamic(this, &UNButtonListViewEntry::OnButtonPressed);
			Button->OnHovered.AddDynamic(this, &UNButtonListViewEntry::OnButtonHovered);
			Button->OnUnhovered.AddDynamic(this, &UNButtonListViewEntry::OnButtonUnhovered);
			Button->OnReleased.AddDynamic(this, &UNButtonListViewEntry::OnButtonReleased);
		}
	}

	virtual void NativeDestruct() override
	{
		if (Button != nullptr && Button->IsValidLowLevel())
		{
			Button->OnClicked.RemoveAll(this);
			Button->OnHovered.RemoveAll(this);
			Button->OnUnhovered.RemoveAll(this);
			Button->OnReleased.RemoveAll(this);
		}
		
		Super::NativeDestruct();
	}
	
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override
	{
		const UNButtonListViewEntryObject* ButtonObject = Cast<UNButtonListViewEntryObject>(ListItemObject);
		if (ButtonObject != nullptr)
		{
			SetText(ButtonObject->GetText());
		}
		Button->OnClicked.AddDynamic(ButtonObject, &UNButtonListViewEntryObject::OnPressed);
		Object = ListItemObject;
		
		OnButtonUnhovered();
	}
	
	virtual void NativeOnEntryReleased() override
	{
		if (Object != nullptr)
		{
			const UNButtonListViewEntryObject* ButtonObject = Cast<UNButtonListViewEntryObject>(Object);
			Button->OnClicked.RemoveAll(ButtonObject);
			Object = nullptr;
		}
	}

	virtual void SetOwnerListView(UObject* Widget, UNListView* Owner) override
	{
		OwnerListView = Owner;
		Execute_OnSetOwnerListView(Widget, Owner);
	}
	
	UFUNCTION(BlueprintCallable)
	void SetText(const FText NewText) const
	{
		Text->SetText(NewText);
	}

protected:	
	
	UFUNCTION()
	void OnButtonPressed()
	{
		if (Object == nullptr) return;
		const UNButtonListViewEntryObject* ButtonObject = Cast<UNButtonListViewEntryObject>(Object);

		if (ButtonObject != nullptr)
		{
			Button->SetBackgroundColor(FNColor::GetLinearColor(ButtonObject->PressedStateBackgroundColor));
			Text->SetColorAndOpacity(FNColor::GetLinearColor(ButtonObject->PressedStateForegroundColor));
		}
	}
	
	UFUNCTION()
	void OnButtonHovered()
	{
		if (Object == nullptr) return;
		const UNButtonListViewEntryObject* ButtonObject = Cast<UNButtonListViewEntryObject>(Object);

		if (ButtonObject != nullptr)
		{
			Button->SetBackgroundColor(FNColor::GetLinearColor(ButtonObject->HoverStateBackgroundColor));
			Text->SetColorAndOpacity(FNColor::GetLinearColor(ButtonObject->HoverStateForegroundColor));
		}
	}
	
	UFUNCTION()
	void OnButtonReleased()
	{
		if (Object == nullptr) return;
		const UNButtonListViewEntryObject* ButtonObject = Cast<UNButtonListViewEntryObject>(Object);

		if (ButtonObject != nullptr)
		{
			Button->SetBackgroundColor(FNColor::GetLinearColor(ButtonObject->HoverStateBackgroundColor));
			Text->SetColorAndOpacity(FNColor::GetLinearColor(ButtonObject->HoverStateForegroundColor));
		}
	}
	
	UFUNCTION()
	void OnButtonUnhovered()
	{
		if (Object == nullptr) return;
		const UNButtonListViewEntryObject* ButtonObject = Cast<UNButtonListViewEntryObject>(Object);

		if (ButtonObject != nullptr)
		{
			Button->SetBackgroundColor(FNColor::GetLinearColor(ButtonObject->UnhoveredStateBackgroundColor));
			Text->SetColorAndOpacity(FNColor::GetLinearColor(ButtonObject->UnhoveredStateForegroundColor));
		}
	}
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNListView> OwnerListView;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UButton> Button;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> Text;

private:

	TObjectPtr<UObject> Object;
};
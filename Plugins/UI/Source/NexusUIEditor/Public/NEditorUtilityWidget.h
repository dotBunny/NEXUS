﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once


#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "NEditorUtilityWidget.generated.h"

UCLASS()
class NEXUSUIEDITOR_API UNEditorUtilityWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	virtual TAttribute<const FSlateBrush*> GetTabDisplayIcon() const { return TAttribute<const FSlateBrush*>(); }
	virtual FText GetTabDisplayText() const { return FText::FromString(TEXT("NEditorUtilityWidget")); }

	void PinTemplate(UEditorUtilityWidgetBlueprint* Template)
	{
		PinnedTemplate = Template;
		PinnedTemplate->AddToRoot();
	}

	void UnpinTemplate()
	{
		if (PinnedTemplate != nullptr)
		{
			PinnedTemplate->RemoveFromRoot();
			PinnedTemplate = nullptr;
		}
	}
	
protected:
	/**
	 * Accessing this has to happen on the following frame after constructing the widget.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D UnitScale = FVector2D::One();	

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(BlueprintReadOnly);
	TObjectPtr<UEditorUtilityWidgetBlueprint> PinnedTemplate;

private:
	UFUNCTION()
	void DelayedConstructTask();
	void UpdateEditorTab(const FName& InTaName) const;
};
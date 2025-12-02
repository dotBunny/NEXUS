// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once


#include "EditorUtilityWidget.h"
#include "NEditorUtilityWidget.h"
#include "Textures/SlateIcon.h"
#include "NWidgetEditorUtilityWidget.generated.h"

// TODO : restore old
// TODO: EUW menu registration name

UCLASS()
class NEXUSUIEDITOR_API UNWidgetEditorUtilityWidget : public UNEditorUtilityWidget
{
	GENERATED_BODY()

public:
	static UNWidgetEditorUtilityWidget* GetOrCreate(const FName Identifier, TSubclassOf<UUserWidget> ContentWidget, const FText& TabDisplayText = FText::GetEmpty(), const FName& TabIconStyle = TEXT(""), const FString& TabIconName = TEXT(""));
	
	static bool HasEditorUtilityWidget(FName Identifier);
	
	virtual FText GetTabDisplayText() const override
	{
		if (BaseWidget != nullptr && BaseWidget->Implements<UNWidgetTabDetails>())
		{
			const INWidgetTabDetails* TabDetails = Cast<INWidgetTabDetails>(BaseWidget);
			return TabDetails->GetTabDisplayText();
		}
		return TabDisplayText;
	}
	virtual TAttribute<const FSlateBrush*> GetTabDisplayIcon() override
	{
		if (BaseWidget != nullptr && BaseWidget->Implements<UNWidgetTabDetails>())
		{
			INWidgetTabDetails* TabDetails = Cast<INWidgetTabDetails>(BaseWidget);
			return TabDetails->GetTabDisplayIcon();
		}
		
		if (!TabIcon.IsSet() && !TabIconStyle.IsNone() && TabIconName.Len() > 0)
		{
			TabIcon = FSlateIcon(TabIconStyle, FName(TabIconName));
		}
		if (TabIcon.IsSet())
		{
			return TabIcon.GetIcon();
		}
		return TAttribute<const FSlateBrush*>();
	}

protected:
	
	virtual void NativeDestruct() override;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UUserWidget> BaseWidget = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	FText TabDisplayText = FText::FromString(TEXT("NEditorUtilityWidget"));
	
	UPROPERTY(BlueprintReadOnly)
	FName TabIconStyle;
	
	UPROPERTY(BlueprintReadOnly)
	FString TabIconName;

private:
	virtual void DelayedConstructTask() override;
	static TMap<FName, UNWidgetEditorUtilityWidget*> KnownEditorUtilityWidgets;
	FSlateIcon TabIcon;
};


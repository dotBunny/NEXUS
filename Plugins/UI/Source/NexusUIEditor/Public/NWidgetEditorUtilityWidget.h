// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetSystem.h"
#include "Textures/SlateIcon.h"
#include "NWidgetEditorUtilityWidget.generated.h"

UCLASS()
class NEXUSUIEDITOR_API UNWidgetEditorUtilityWidget : public UNEditorUtilityWidget
{
	GENERATED_BODY()

public:
	
	static const FString TemplatePath;
	
	static UNWidgetEditorUtilityWidget* GetOrCreate(const FName Identifier, const FString& WidgetBlueprint, const FText& TabDisplayText = FText::GetEmpty(), const FName& TabIconStyle = TEXT(""), const FString& TabIconName = TEXT(""));
	
	static bool HasEditorUtilityWidget(FName Identifier);
	static UNWidgetEditorUtilityWidget* GetEditorUtilityWidget(FName Identifier);
	
	virtual FText GetTabDisplayText() const override;
	
	virtual FSlateIcon GetTabDisplayIcon() override;

	virtual TAttribute<const FSlateBrush*> GetTabDisplayBrush() override;
	
	virtual FName GetTabIdentifier() override
	{
		if (PinnedTemplate != nullptr)
		{
			return PinnedTemplate->GetRegistrationName();
		}
		return WidgetIdentifier;
	}
	virtual FName GetUserSettingsIdentifier() override
	{
		return WidgetIdentifier;
	}
	
	static FNWidgetState CreateWidgetState(const FString& WidgetBlueprint, const FText& TabDisplayText, const FName& TabIconStyle, const FString& TabIconName);
	
	virtual FString GetUserSettingsTemplate() override
	{
		return TemplatePath;
	}
	

	

	//~INWidgetStateProvider interface
	virtual FNWidgetState GetWidgetState(UObject* BlueprintWidget) override;
	virtual void RestoreWidgetState(UObject* BlueprintWidget, FName Identifier, FNWidgetState& WidgetState) override;
	//~End of INWidgetStateProvider interface

protected:
	
	virtual void NativeDestruct() override;
	
	UPROPERTY(BlueprintReadOnly)
	FName WidgetIdentifier = GetFName();
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UUserWidget> BaseWidget = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	FString WidgetBlueprint;
	
	UPROPERTY(BlueprintReadOnly)
	FText TabDisplayText = FText::FromString(TEXT("NEditorUtilityWidget"));
	
	UPROPERTY(BlueprintReadOnly)
	FName TabIconStyle;
	
	UPROPERTY(BlueprintReadOnly)
	FString TabIconName;

private:
	virtual void DelayedConstructTask() override;
	static TMap<FName, UNWidgetEditorUtilityWidget*> KnownWidgets;
	FSlateIcon TabIcon;
};


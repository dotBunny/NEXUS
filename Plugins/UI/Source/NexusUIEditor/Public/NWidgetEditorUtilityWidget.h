// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonBorder.h"
#include "CommonTextBlock.h"
#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetSystem.h"
#include "Textures/SlateIcon.h"
#include "NWidgetEditorUtilityWidget.generated.h"

// TODO The child BP has no idea about recompile and crashes?

UCLASS()
class NEXUSUIEDITOR_API UNWidgetEditorUtilityWidget : public UNEditorUtilityWidget
{
	GENERATED_BODY()

public:
	
	static const FString TemplatePath;
	
	static UNWidgetEditorUtilityWidget* GetOrCreate(const FName Identifier, const FString& WidgetBlueprint, const FText& TabDisplayText = FText::GetEmpty(), const FName& TabIconStyle = TEXT(""), const FString& TabIconName = TEXT(""));
	
	static bool HasWidget(FName Identifier);
	static UNWidgetEditorUtilityWidget* GetWidget(FName Identifier);
	
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
	
	
	void UpdateHeader() const;

protected:
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonBorder> Header;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> HeaderText;
	
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
	
	UPROPERTY()
	TObjectPtr<UWidgetBlueprint> ContentWidgetTemplate;
	
	void OnBlueprintPreCompile(UBlueprint* Blueprint);
	
private:
	virtual void DelayedConstructTask() override;
	static TMap<FName, UNWidgetEditorUtilityWidget*> KnownWidgets;
	FSlateIcon TabIcon;
	FDelegateHandle OnBlueprintPreCompileHandle;
	FDelegateHandle OnStatusProviderUpdateHandle;
};


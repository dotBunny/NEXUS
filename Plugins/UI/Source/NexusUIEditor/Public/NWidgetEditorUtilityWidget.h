// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once


#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetComponents.h"
#include "NCoreEditorMinimal.h"
#include "NEditorUtilityWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Textures/SlateIcon.h"
#include "NWidgetEditorUtilityWidget.generated.h"

// TODO : restore old
// TODO: EUW menu registration name

UCLASS()
class NEXUSUIEDITOR_API UNWidgetEditorUtilityWidget : public UNEditorUtilityWidget
{
	GENERATED_BODY()

public:
	
	static const FString TemplatePath;
	
	static UNWidgetEditorUtilityWidget* GetOrCreate(const FName Identifier, const FString& WidgetBlueprint, const FText& TabDisplayText = FText::GetEmpty(), const FName& TabIconStyle = TEXT(""), const FString& TabIconName = TEXT(""));
	
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

	virtual FNEditorUtilityWidgetPayload GetUserSettingsPayload() override
	{
		return CreatePayload(WidgetBlueprint, TabDisplayText, TabIconStyle, TabIconName);
	}
	
	static FNEditorUtilityWidgetPayload CreatePayload(const FString& WidgetBlueprint, const FText& TabDisplayText, const FName& TabIconStyle, const FString& TabIconName)
	{
		FNEditorUtilityWidgetPayload Payload;
		
		Payload.Strings.Add(WidgetBlueprint);
		Payload.Strings.Add(TabDisplayText.ToString());
		Payload.Strings.Add(TabIconStyle.ToString());
		Payload.Strings.Add(TabIconName);
		
		return Payload;
	}
	
	virtual FString GetUserSettingsTemplate() override
	{
		return TemplatePath;
	}

	virtual void RestoreFromUserSettingsPayload(FName Identifier, FNEditorUtilityWidgetPayload Payload) override
	{
		WidgetIdentifier = Identifier;
		WidgetBlueprint = Payload.Strings[0];
		TabDisplayText = FText::FromString(Payload.Strings[1]);
		TabIconStyle = FName(Payload.Strings[2]);
		TabIconName = Payload.Strings[3];
		
		const UWidgetBlueprint* ContentWidgetTemplate = LoadObject<UWidgetBlueprint>(nullptr, WidgetBlueprint);
		const TSubclassOf<UUserWidget> ContentWidget{ContentWidgetTemplate->GeneratedClass};
			
		if (ContentWidget != nullptr)
		{
			BaseWidget = WidgetTree->ConstructWidget(ContentWidget);
				
			UEditorUtilityScrollBox* EditorScrollBox = Cast<UEditorUtilityScrollBox>(WidgetTree->RootWidget);
			EditorScrollBox->AddChild(BaseWidget);
		}
		else
		{
			WidgetBlueprint = TEXT("");
			NE_LOG(Warning, TEXT("[UNWidgetEditorUtilityWidget::RestoreFromUserSettingsPayload] Unable to find content widget to use for template."), *TemplatePath)
		}
		
		OnRestoreFromUserSettingsPayload(Identifier, Payload);
	}
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
	static TMap<FName, UNWidgetEditorUtilityWidget*> KnownEditorUtilityWidgets;
	FSlateIcon TabIcon;
};


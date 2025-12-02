// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once


#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "INWidgetTabDetails.h"
#include "NEditorUtilityWidgetUserSettings.h"
#include "NEditorUtilityWidget.generated.h"

/**
 * An extension on the UEditorUtilityWidget providing additional functionality around customization and appearance.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/editor-types/editor-utility-widget/">UNEditorUtilityWidget</a>
 */
UCLASS()
class NEXUSUIEDITOR_API UNEditorUtilityWidget : public UEditorUtilityWidget, public INWidgetTabDetails
{
	GENERATED_BODY()

public:
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
	virtual void RestoreFromUserSettingsPayload(FName Identifier, FNEditorUtilityWidgetUserSettingsPayload Payload) { OnRestoreFromUserSettingsPayload(Identifier, Payload); };
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShouldSerializeWidget = true;
	
	/**
	 * Accessing this has to happen on the following frame after constructing the widget.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D UnitScale = FVector2D::One();	

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;	
	
	virtual FNEditorUtilityWidgetUserSettingsPayload GetUserSettingsPayload() { return OnGetUserSettingsPayload(); };
	
	virtual FName GetTabIdentifier()
	{
		if (PinnedTemplate != nullptr)
		{
			return PinnedTemplate->GetRegistrationName();
		}
		return GetFName();
	}
	virtual FName GetUserSettingsIdentifier()
	{
		if (PinnedTemplate != nullptr)
		{
			return PinnedTemplate->GetRegistrationName();
		}
		return GetFName();
	}
	virtual FString GetUserSettingsTemplate()
	{
		if (PinnedTemplate != nullptr)
		{
			return PinnedTemplate->GetFullName();
		}
		return GetFullName();
	}
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnRestoreFromUserSettingsPayload(FName& Identifier, FNEditorUtilityWidgetUserSettingsPayload& Payload);
	
	UFUNCTION(BlueprintImplementableEvent)
	FNEditorUtilityWidgetUserSettingsPayload OnGetUserSettingsPayload();
	
	UPROPERTY(BlueprintReadOnly);
	TObjectPtr<UEditorUtilityWidgetBlueprint> PinnedTemplate;

	UFUNCTION()
	virtual void DelayedConstructTask();
	
	void OnTabClosed(TSharedRef<SDockTab> Tab);

private:
	SDockTab::FOnTabClosedCallback OnTabClosedCallback;
};
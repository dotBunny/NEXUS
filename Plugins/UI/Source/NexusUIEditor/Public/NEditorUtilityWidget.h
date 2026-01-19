// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once


#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "INWidgetStateProvider.h"
#include "INWidgetTabDetailsProvider.h"
#include "NEditorUtilityWidget.generated.h"

/**
 * An extension on the UEditorUtilityWidget providing additional functionality around customization and appearance.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/editor-types/editor-utility-widget/">UNEditorUtilityWidget</a>
 */
UCLASS()
class NEXUSUIEDITOR_API UNEditorUtilityWidget : public UEditorUtilityWidget, public INWidgetTabDetailsProvider, public INWidgetStateProvider
{
	GENERATED_BODY()

public:
	static const FString WidgetState_WidgetBlueprint;
	static const FString WidgetState_TabDisplayText;
	static const FString WidgetState_TabIconStyle;
	static const FString WidgetState_TabIconName;
	
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
	
	FNWidgetState GetState()
	{
		return GetWidgetState(this);
	};
	
	bool IsPersistent() const
	{
		return bIsPersistent;
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

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsPersistent = false;
	
	/**
	 * Accessing this has to happen on the following frame after constructing the widget.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D UnitScale = FVector2D::One();	

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;	
	
	virtual FName GetTabIdentifier()
	{
		if (PinnedTemplate != nullptr)
		{
			return PinnedTemplate->GetRegistrationName();
		}
		return GetFName();
	}
	
	UPROPERTY(BlueprintReadOnly);
	TObjectPtr<UEditorUtilityWidgetBlueprint> PinnedTemplate;
	
	
	UFUNCTION()
	virtual void DelayedConstructTask();
	
	void OnTabClosed(TSharedRef<SDockTab> Tab);


private:
	SDockTab::FOnTabClosedCallback OnTabClosedCallback;
};
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
	
	void SetTemplate(TObjectPtr<UEditorUtilityWidgetBlueprint> Template);
	
	UFUNCTION(BlueprintCallable)
	bool IsPersistent() const;
	
	virtual FName GetUserSettingsIdentifier();
	virtual FString GetUserSettingsTemplate();
	
protected:
	
	virtual void NativeConstruct() override;
	
	virtual void NativeDestruct() override;	
	
	UFUNCTION()
	virtual void DelayedConstructTask();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsPersistent = false;

	UPROPERTY()
	TObjectPtr<UEditorUtilityWidgetBlueprint> PinnedTemplate;
	
	/**
	 * Accessing this has to happen on the following frame after constructing the widget.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D UnitScale = FVector2D::One();	

private:
	void OnTabClosed(TSharedRef<SDockTab> Tab);
	
	SDockTab::FOnTabClosedCallback OnTabClosedCallback;
};
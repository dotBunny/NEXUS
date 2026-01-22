// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorUtilityWidget.h"
#include "INWidgetStateProvider.h"
#include "NEditorUtilityWidget.generated.h"

/**
 * An extension on the UEditorUtilityWidget providing additional functionality around customization and appearance.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/editor-types/editor-utility-widget/">UNEditorUtilityWidget</a>
 */
UCLASS(BlueprintType)
class NEXUSUIEDITOR_API UNEditorUtilityWidget : public UEditorUtilityWidget, public INWidgetStateProvider
{
	GENERATED_BODY()

public:
	
	static UEditorUtilityWidget* SpawnTab(const FString& ObjectPath, FName Identifier = NAME_None);
	
	UFUNCTION(BlueprintCallable)
	bool IsPersistent() const { return bIsPersistent; };
	FName GetStateIdentifier() const { return StateIdentifier; };
	
protected:
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	virtual void DelayedConstructTask();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="State")
	bool bIsPersistent = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="State")
	FName StateIdentifier;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tab")
	FName TabIconStyle;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tab")
	FName TabIconName;
		
	/**
	 * Accessing this has to happen on the following frame after constructing the widget.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Info")
	FVector2D UnitScale = FVector2D::One();	
	
private:
	void OnTabClosed(TSharedRef<SDockTab> Tab);
	SDockTab::FOnTabClosedCallback OnTabClosedCallback;
	FSlateIcon TabIcon;
};
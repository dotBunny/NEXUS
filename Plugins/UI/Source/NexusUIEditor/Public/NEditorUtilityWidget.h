// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorUtilityLibrary.h"
#include "EditorUtilityWidget.h"
#include "NEditorUtilityWidget.generated.h"

/**
 * An extension on the UEditorUtilityWidget providing additional functionality around customization and appearance.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/editor-types/editor-utility-widget/">UNEditorUtilityWidget</a>
 */
UCLASS(BlueprintType)
class NEXUSUIEDITOR_API UNEditorUtilityWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	
	static UEditorUtilityWidget* SpawnTab(const FString& ObjectPath, FName Identifier = NAME_None);
	
	UFUNCTION(BlueprintCallable)
	bool IsPersistent() const
	{
		return bIsPersistent;
	};
	
	UFUNCTION(BlueprintCallable)
	FName GetUniqueIdentifier() const
	{
		return UniqueIdentifier;
	};
	
	UFUNCTION(BlueprintCallable)
	FName GetTabIdentifier() const
	{
		return CachedTabIdentifier;
	};
	
protected:
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	virtual void DelayedConstructTask();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="State")
	bool bIsPersistent = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="State", 
		meta=(Tooltip="Should the widget attempt to remove itself when the tab is closed from any cached state in the UNEditorUtilityWidgetSubsystem."))
	bool bHasPermanentState = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="State")
	FName UniqueIdentifier;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tab")
	FName TabIconStyle;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tab")
	FName TabIconName;
		
	/**
	 * Accessing this has to happen on the following frame after constructing the widget.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Info")
	FVector2D UnitScale = FVector2D::One();	

	UPROPERTY()
	TObjectPtr<UAsyncEditorDelay> DelayedTask;
	
private:
	void OnTabClosed(TSharedRef<SDockTab> Tab);
	SDockTab::FOnTabClosedCallback OnTabClosedCallback;
	FSlateIcon TabIcon;
	FName CachedTabIdentifier;
	
};
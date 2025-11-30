// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once


#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "NEditorUtilityWidget.h"
#include "NUIEditorStyle.h"
#include "Textures/SlateIcon.h"
#include "NWidgetEditorUtilityWidget.generated.h"

// TODO : restore old, rename to wrapper?

/**
 * An extension on the UEditorUtilityWidget providing additional functionality around customization and appearance.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/editor-types/editor-utility-widget/">UNEditorUtilityWidget</a>
 */
UCLASS()
class NEXUSUIEDITOR_API UNWidgetEditorUtilityWidget : public UNEditorUtilityWidget
{
	GENERATED_BODY()

public:
	/// TODO: GetOrCreateFromWidget
	/// TODO: GetEditorUtilityWidgetOf
	/// TODO: Add title / icon? 
	static UNWidgetEditorUtilityWidget* GetOrCreate(const FName Identifier, TSubclassOf<UUserWidget> ContentWidget, const FText& InitialTabDisplayText);
	
	static bool HasEditorUtilityWidget(FName Identifier);
	

	virtual FText GetTabDisplayText() const override { return TabDisplayText; }

protected:
	
	virtual void NativeDestruct() override;

	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UUserWidget> BaseWidget = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	FText TabDisplayText = FText::FromString(TEXT("NEditorUtilityWidget"));
	
	UPROPERTY(BlueprintReadOnly)
	FString TabIconStyle;
	
	UPROPERTY(BlueprintReadOnly)
	FString TabIconName;
	FSlateIcon TabIcon;

private:
	virtual void DelayedConstructTask() override;
	
	static TMap<FName, UNWidgetEditorUtilityWidget*> KnownEditorUtilityWidgets;
};


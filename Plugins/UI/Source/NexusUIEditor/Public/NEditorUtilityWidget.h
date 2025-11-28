// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once


#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "NUIEditorStyle.h"
#include "Textures/SlateIcon.h"
#include "NEditorUtilityWidget.generated.h"

/**
 * An extension on the UEditorUtilityWidget providing additional functionality around customization and appearance.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/editor-types/editor-utility-widget/">UNEditorUtilityWidget</a>
 */
UCLASS()
class NEXUSUIEDITOR_API UNEditorUtilityWidget : public UEditorUtilityWidget
{
	static TCHAR* WidgetTemplate;
	
	GENERATED_BODY()

public:
	/// TODO: GetOrCreateFromWidget
	/// TODO: GetEditorUtilityWidgetOf
	/// TODO: Add title / icon? 
	static UNEditorUtilityWidget* CreateFromWidget(TSubclassOf<UUserWidget> ContentWidget, FName NameOverride = NAME_None);
	static bool HasEditorUtilityWidgetByName(FName Name);
	
	virtual TAttribute<const FSlateBrush*> GetTabDisplayIcon() const { return TAttribute<const FSlateBrush*>(); }
	//virtual TAttribute<const FSlateBrush*> GetTabDisplayIcon() const { return TabIcon.GetIcon(); }
	virtual FText GetTabDisplayText() const { return TabDisplayText; }

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
	
protected:
	/**
	 * Accessing this has to happen on the following frame after constructing the widget.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D UnitScale = FVector2D::One();	

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UEditorUtilityWidgetBlueprint> PinnedTemplate;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UUserWidget> BaseWidget = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	FText TabDisplayText = FText::FromString(TEXT("NEditorUtilityWidget"));
	
	// UPROPERTY(BlueprintReadOnly)
	// FSlateIcon TabIcon = FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.SelectActor");

private:
	UFUNCTION()
	void DelayedConstructTask();
	void UpdateEditorTab(const FName& InTaName) const;
	
	static TMap<FName, UNEditorUtilityWidget*> KnownEditorUtilityWidgets;
};


// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorUtilityWidget.h"
#include "NProcGenEditorUtilityWidget.generated.h"

class UMenuAnchor;

UCLASS()
class UNProcGenEditorUtilityWidget : public UNEditorUtilityWidget
{
	GENERATED_BODY()

public:
	static void CreateWidget(const FToolMenuContext& InContext);
	static void CreateWidget();
	static bool CreateWidget_CanExecute(const FToolMenuContext& InContext);
	static bool CreateWidget_CanExecute();
	
	static void AddEntryToToolMenu(FToolMenuSection& InMenuSection);
	
	static FText Label;
	static FText FullLabel;
	static FText Tooltip;

	static UNProcGenEditorUtilityWidget* Instance;

	virtual TAttribute<const FSlateBrush*> GetTabDisplayIcon() const override { return TabIcon.GetIcon(); }
	virtual FText GetTabDisplayText() const override { return TabDisplayText;  }
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
private:
		
	FSlateIcon TabIcon;
	FText TabDisplayText;
};

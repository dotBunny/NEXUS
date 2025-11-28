// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorUtilityWidget.h"
#include "NProcGenEditorStyle.h"
#include "NProcGenSettings.h"

// TODO: Move to MENU system

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

FText UNProcGenEditorUtilityWidget::Label = LOCTEXT("NProcGenSystemEditorUtilityWidget_Open", "Open NProcGenSystem Window");
FText UNProcGenEditorUtilityWidget::FullLabel = LOCTEXT("NProcGenSystemEditorUtilityWidget_OpenFull", "NProcGenSystem");
FText UNProcGenEditorUtilityWidget::Tooltip = LOCTEXT("NProcGenSystemEditorUtilityWidget_Tip", "Opens the NProcGenSystem Window inside of an editor tab.");

UNProcGenEditorUtilityWidget* UNProcGenEditorUtilityWidget::Instance = nullptr;

void UNProcGenEditorUtilityWidget::CreateWidget(const FToolMenuContext& InContext)
{
	CreateWidget();
}

void UNProcGenEditorUtilityWidget::CreateWidget()
{
	TSubclassOf<UNProcGenDeveloperOverlayWidget> WidgetClass = UNProcGenSettings::Get()->DeveloperOverlayWidget;
	if (WidgetClass == nullptr)
	{
		const FString TemplatePath = FString::Printf(TEXT("/Script/UMGEditor.WidgetBlueprint'/NexusProcGen/WB_NProcGenDeveloperOverlay.WB_NProcGenDeveloperOverlay'"));
		UWidgetBlueprint* TemplateWidget = LoadObject<UWidgetBlueprint>(nullptr, TemplatePath);
		
		WidgetClass = TemplateWidget->GeneratedClass;
	}
	CreateFromWidget(WidgetClass);
}

bool UNProcGenEditorUtilityWidget::CreateWidget_CanExecute(const FToolMenuContext& InContext)
{
	return CreateWidget_CanExecute();
}

bool UNProcGenEditorUtilityWidget::CreateWidget_CanExecute()
{
	return Instance == nullptr;
}

void UNProcGenEditorUtilityWidget::AddEntryToToolMenu(FToolMenuSection& InMenuSection)
{
	FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry("NEXUS.ProcGen.System", FullLabel,Tooltip,
		FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Icon.ProcGen"),
FUIAction(
	FExecuteAction::CreateStatic(&CreateWidget),
	FCanExecuteAction::CreateStatic(&CreateWidget_CanExecute),
	FIsActionChecked(),
	FIsActionButtonVisible()));
	InMenuSection.AddEntry(Entry);
}


void UNProcGenEditorUtilityWidget::NativeConstruct()
{
	Instance = this;
	TabIcon = FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Icon.ProcGen");
	TabDisplayText = FText::FromString(TEXT("NProcGen System"));
	Super::NativeConstruct();
}

void UNProcGenEditorUtilityWidget::NativeDestruct()
{
	Instance = nullptr;
	Super::NativeDestruct();
}

#undef LOCTEXT_NAMESPACE

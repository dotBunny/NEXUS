// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenSystemEditorUtilityWidget.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "NProcGenEditorStyle.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

FText UNProcGenSystemEditorUtilityWidget::Label = LOCTEXT("NProcGenSystemEditorUtilityWidget_Open", "Open NProcGenSystem Window");
FText UNProcGenSystemEditorUtilityWidget::FullLabel = LOCTEXT("NProcGenSystemEditorUtilityWidget_OpenFull", "NProcGenSystem");
FText UNProcGenSystemEditorUtilityWidget::Tooltip = LOCTEXT("NProcGenSystemEditorUtilityWidget_Tip", "Opens the NProcGenSystem Window inside of an editor tab.");

UNProcGenSystemEditorUtilityWidget* UNProcGenSystemEditorUtilityWidget::Instance = nullptr;

void UNProcGenSystemEditorUtilityWidget::CreateWidget(const FToolMenuContext& InContext)
{
	CreateWidget();
}

void UNProcGenSystemEditorUtilityWidget::CreateWidget()
{
	// // TODO: Path to blueprint ? OR can we inject our own defined widget?
	// // 
	// // inline const TCHAR* PATH =
	// // 		L"/Script/Blutility.EditorUtilityWidgetBlueprint'NexusProcGen/EUW_ProcGen.EUW_ProcGen'";
	// UBlueprint* TimelineWidget = LoadObject<UBlueprint>(nullptr, ForgeFrameworkPaths::Timeline::EditorDebugWindow);
	//
	// if (UEditorUtilityWidgetBlueprint* EditorWidget = Cast<UEditorUtilityWidgetBlueprint>(TimelineWidget))
	// {
	// 	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	// 	UEditorUtilityWidget* Widget = EditorUtilitySubsystem->SpawnAndRegisterTab(EditorWidget);
	// 	UNProcGenSystemEditorUtilityWidget* EUW = Cast<UNProcGenSystemEditorUtilityWidget>(Widget);
	// 	EUW->PinTemplate(EditorWidget);
	// }
}

bool UNProcGenSystemEditorUtilityWidget::CreateWidget_CanExecute(const FToolMenuContext& InContext)
{
	return CreateWidget_CanExecute();
}

bool UNProcGenSystemEditorUtilityWidget::CreateWidget_CanExecute()
{
	return Instance == nullptr;
}

void UNProcGenSystemEditorUtilityWidget::AddEntryToToolMenu(FToolMenuSection& InMenuSection)
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


void UNProcGenSystemEditorUtilityWidget::NativeConstruct()
{
	Instance = this;
	TabIcon = FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Icon.ProcGen");
	TabDisplayText = FText::FromString(TEXT("NProcGen System"));
	Super::NativeConstruct();
}

void UNProcGenSystemEditorUtilityWidget::NativeDestruct()
{
	Instance = nullptr;
	Super::NativeDestruct();
}

#undef LOCTEXT_NAMESPACE

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWidgetEditorUtilityWidget.h"

#include "NCoreEditorMinimal.h"

TMap<FName, UNWidgetEditorUtilityWidget*> UNWidgetEditorUtilityWidget:: KnownEditorUtilityWidgets;
const FString UNWidgetEditorUtilityWidget::TemplatePath = TEXT("/Script/Blutility.EditorUtilityWidgetBlueprint'/NexusUI/EditorResources/EUW_NWidgetWrapper.EUW_NWidgetWrapper'");

UNWidgetEditorUtilityWidget* UNWidgetEditorUtilityWidget::GetOrCreate(const FName Identifier, const FString& WidgetBlueprint, const FText& TabDisplayText,  const FName& TabIconStyle, const FString& TabIconName)
{
	if (Identifier == NAME_None)
	{
		NE_LOG(Error, TEXT("[UNEditorUtilityWidget::GetOrCreate] A proper Identifier must be provided for the EUW."))
		return nullptr;
	}
	
	// Return existing
	if (HasEditorUtilityWidget(Identifier))
	{
		return KnownEditorUtilityWidgets[Identifier];
	}
	
	
	const FNEditorUtilityWidgetPayload Payload = CreatePayload(WidgetBlueprint, TabDisplayText, TabIconStyle, TabIconName);
	UNEditorUtilityWidget* Widget = UNEditorUtilityWidgetSystem::CreateWithPayload(TemplatePath, Identifier, Payload);
	return Cast<UNWidgetEditorUtilityWidget>(Widget);
	
	//
	// // Looks like we need to make it
	// const UBlueprint* TemplateWidget = LoadObject<UBlueprint>(nullptr, TemplatePath);
	// if (TemplateWidget == nullptr)
	// {
	// 	NE_LOG(Error, TEXT("[UNEditorUtilityWidget::GetOrCreate] Unable to load template blueprint. (%s)"), *TemplatePath)
	// }
	//
	// // Need to duplicate the base
	// UBlueprint* TemplateDuplicate = DuplicateObject<UBlueprint>(TemplateWidget, TemplateWidget->GetOutermost(), Identifier);
	// TemplateDuplicate->SetFlags(RF_Public | RF_Transient | RF_TextExportTransient | RF_DuplicateTransient);
	//
	// if (UEditorUtilityWidgetBlueprint* EditorWidget = Cast<UEditorUtilityWidgetBlueprint>(TemplateDuplicate))
	// {
	// 	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	// 	UEditorUtilityWidget* Widget = EditorUtilitySubsystem->SpawnAndRegisterTab(EditorWidget);
	// 	
	// 	// We dont want these tracked
	// 	IBlutilityModule* BlutilityModule = FModuleManager::GetModulePtr<IBlutilityModule>("Blutility");
	// 	BlutilityModule->RemoveLoadedScriptUI(EditorWidget);
	// 	
	// 	if (UNWidgetEditorUtilityWidget* UtilityWidget = Cast<UNWidgetEditorUtilityWidget>(Widget); 
	// 		UtilityWidget != nullptr)
	// 	{
	// 		UtilityWidget->PinTemplate(EditorWidget);
	//
	// 		UtilityWidget->RestoreFromUserSettingsPayload(Identifier, Payload);
	// 	
	// 		return UtilityWidget;
	// 	}
	//
	// 	NE_LOG(Error, TEXT("[UNEditorUtilityWidget::GetOrCreate] Unable to cast to UNEditorUtilityWidget. (%s)"), *TemplatePath)
	// }
	// else
	// {
	// 	NE_LOG(Error, TEXT("[UNEditorUtilityWidget::GetOrCreate] Template is not a UEditorUtilityWidgetBlueprint. (%s)"), *TemplatePath)
	// }
	// return nullptr;
}

bool UNWidgetEditorUtilityWidget::HasEditorUtilityWidget(const FName Identifier)
{
	return KnownEditorUtilityWidgets.Contains(Identifier);
}

void UNWidgetEditorUtilityWidget::NativeDestruct()
{
	KnownEditorUtilityWidgets.Remove(WidgetIdentifier);
	Super::NativeDestruct();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
void UNWidgetEditorUtilityWidget::DelayedConstructTask()
{
	// We register a frame later as the base widget data has been set at that point
	KnownEditorUtilityWidgets.Add(WidgetIdentifier, this);
	Super::DelayedConstructTask();
}
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWidgetEditorUtilityWidget.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilityWidgetComponents.h"
#include "NCoreEditorMinimal.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"

TMap<FName, UNWidgetEditorUtilityWidget*> UNWidgetEditorUtilityWidget:: KnownEditorUtilityWidgets;

UNWidgetEditorUtilityWidget* UNWidgetEditorUtilityWidget::GetOrCreate(const FName Identifier, const TSubclassOf<UUserWidget> ContentWidget, const FText& TabDisplayText,  const FName& TabIconStyle, const FString& TabIconName)
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
	
	// Looks like we need to make it
	const FString TemplatePath = TEXT("/Script/Blutility.EditorUtilityWidgetBlueprint'/NexusUI/EditorResources/EUW_NWidgetWrapper.EUW_NWidgetWrapper'");
	UBlueprint* TemplateWidget = LoadObject<UBlueprint>(nullptr, TemplatePath);
	if (TemplateWidget == nullptr)
	{
		NE_LOG(Error, TEXT("[UNEditorUtilityWidget::GetOrCreate] Unable to load template blueprint. (%s)"), *TemplatePath)
	}

	// Need to duplicate the base
	UBlueprint* TemplateDuplicate = DuplicateObject<UBlueprint>(TemplateWidget, TemplateWidget->GetOutermost(), Identifier);
	TemplateDuplicate->SetFlags(RF_Public | RF_Transient | RF_TextExportTransient | RF_DuplicateTransient);
	
	if (UEditorUtilityWidgetBlueprint* EditorWidget = Cast<UEditorUtilityWidgetBlueprint>(TemplateDuplicate))
	{
		// Check the Nomad flag is set so that we can access by ID
		if (!EditorWidget->ShouldSpawnAsNomadTab())
		{
			NE_LOG(Error, TEXT("[UNEditorUtilityWidget::GetOrCreate] The wrapping widget must have Class Settings -> Settings -> Spawn As Nomad Tab checked (%s)"), *TemplatePath);
			return nullptr;
		}
		
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		UEditorUtilityWidget* Widget = EditorUtilitySubsystem->SpawnAndRegisterTab(EditorWidget);
		if (UNWidgetEditorUtilityWidget* UtilityWidget = Cast<UNWidgetEditorUtilityWidget>(Widget); 
			UtilityWidget != nullptr)
		{
			UtilityWidget->PinTemplate(EditorWidget);

			// Assign defaults
			UtilityWidget->TabDisplayText = TabDisplayText;
			UtilityWidget->TabIconStyle = TabIconStyle;
			UtilityWidget->TabIconName = TabIconName;
			
			if (ContentWidget != nullptr)
			{
				UtilityWidget->BaseWidget = UtilityWidget->WidgetTree->ConstructWidget(ContentWidget, Identifier);
				UEditorUtilityScrollBox* EditorScrollBox = Cast<UEditorUtilityScrollBox>(UtilityWidget->WidgetTree->RootWidget);
				EditorScrollBox->AddChild(UtilityWidget->BaseWidget);
			}
			else
			{
				NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::GetOrCreate] Unable to find content widget to use for template."), *TemplatePath)
			}
			return UtilityWidget;
		}
	
		NE_LOG(Error, TEXT("[UNEditorUtilityWidget::GetOrCreate] Unable to cast to UNEditorUtilityWidget. (%s)"), *TemplatePath)
	}
	else
	{
		NE_LOG(Error, TEXT("[UNEditorUtilityWidget::GetOrCreate] Template is not a UEditorUtilityWidgetBlueprint. (%s)"), *TemplatePath)
	}
	return nullptr;
}

bool UNWidgetEditorUtilityWidget::HasEditorUtilityWidget(const FName Identifier)
{
	return KnownEditorUtilityWidgets.Contains(Identifier);
}


void UNWidgetEditorUtilityWidget::NativeDestruct()
{
	if (BaseWidget != nullptr)
	{
		KnownEditorUtilityWidgets.Remove(BaseWidget->GetFName());
	}
	else
	{
		KnownEditorUtilityWidgets.Remove(GetFName());
	}
	
	Super::NativeDestruct();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
void UNWidgetEditorUtilityWidget::DelayedConstructTask()
{
	// We register a frame later as the base widget data has been set at that point
	if (BaseWidget != nullptr)
	{
		KnownEditorUtilityWidgets.Add(BaseWidget->GetFName(), this);
	}
	else
	{
		KnownEditorUtilityWidgets.Add(GetFName(), this);
	}
	
	Super::DelayedConstructTask();
}
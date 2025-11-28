// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorUtilityWidget.h"

#include "EditorUtilityLibrary.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilityWidgetComponents.h"
#include "NCoreEditorMinimal.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"

TMap<FName, UNEditorUtilityWidget*> UNEditorUtilityWidget:: KnownEditorUtilityWidgets;

// TODO: Handle icon  / tab display name? 
UNEditorUtilityWidget* UNEditorUtilityWidget::GetOrCreate(const FName UniqueIdentifier, const TSubclassOf<UUserWidget> ContentWidget, const FText& InitialTabDisplayText)
{
	if (UniqueIdentifier == NAME_None)
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::CreateFromWidget] A proper Identifier must be provided for the EUW."))
		return nullptr;
	}
	
	if (HasEditorUtilityWidget(UniqueIdentifier))
	{
		return KnownEditorUtilityWidgets[UniqueIdentifier];
	}
	

	// Looks like we need to make it
	const FString TemplatePath = TEXT("/Script/Blutility.EditorUtilityWidgetBlueprint'/NexusUI/EditorResources/EUW_NWidgetTemplate.EUW_NWidgetTemplate'");
	UBlueprint* TemplateWidget = LoadObject<UBlueprint>(nullptr, TemplatePath);
	if (TemplateWidget == nullptr)
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::CreateFromWidget] Unable to load template blueprint. (%s)"), *TemplatePath)
	}

	// TODO: Need to create a copy of the widget/ and give it a different name
	
	// // Need to duplicate the base
	// UBlueprint* TemplateDuplicate = DuplicateObject<UBlueprint>(TemplateWidget, TemplateWidget->GetOutermost(),
	// 	MakeUniqueObjectName(TemplateWidget->GetOutermost(), TemplateWidget->GeneratedClass, NameOverride));
	// TemplateDuplicate->SetFlags(RF_Transactional);
	
	// TODO: We might need to duplicate the template object? as it seems to add to it...need a second window to test
	if (UEditorUtilityWidgetBlueprint* EditorWidget = Cast<UEditorUtilityWidgetBlueprint>(TemplateWidget))
	{
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		UEditorUtilityWidget* Widget = EditorUtilitySubsystem->SpawnAndRegisterTabWithId(EditorWidget, UniqueIdentifier);

		if (UNEditorUtilityWidget* UtilityWidget = Cast<UNEditorUtilityWidget>(Widget); 
			UtilityWidget != nullptr)
		{
			UtilityWidget->PinTemplate(EditorWidget);
			UtilityWidget->TabDisplayText = InitialTabDisplayText;
			
			if (ContentWidget != nullptr)
			{

				// TODO: Should Identifier Get _Widget
				UtilityWidget->BaseWidget = UtilityWidget->WidgetTree->ConstructWidget(ContentWidget, UniqueIdentifier);
				
				// Add to container
				UEditorUtilityScrollBox* EditorScrollBox = Cast<UEditorUtilityScrollBox>(UtilityWidget->WidgetTree->RootWidget);
				EditorScrollBox->AddChild(UtilityWidget->BaseWidget);
			}
			else
			{
				NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::CreateFromWidget] Unable to find content widget to use for template."), *TemplatePath)
			}
			return UtilityWidget;
		}
	
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::CreateFromWidget] Unable to cast to UNEditorUtilityWidget. (%s)"), *TemplatePath)
	}
	else
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::CreateFromWidget] Template is not a UEditorUtilityWidgetBlueprint. (%s)"), *TemplatePath)
	}
	return nullptr;
}

bool UNEditorUtilityWidget::HasEditorUtilityWidget(const FName Identifier)
{
	return KnownEditorUtilityWidgets.Contains(Identifier);
}

void UNEditorUtilityWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// We need to ensure that the window has its icon after all -- this oddly only executes once if you are opening multiple windows at once.
	UAsyncEditorDelay* DelayedConstructTask = NewObject<UAsyncEditorDelay>();
	DelayedConstructTask->Complete.AddDynamic(this, &UNEditorUtilityWidget::DelayedConstructTask);
	DelayedConstructTask->Start(0.01f, 1);
}

void UNEditorUtilityWidget::NativeDestruct()
{
	if (PinnedTemplate != nullptr)
	{
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		EditorUtilitySubsystem->UnregisterTabByID(PinnedTemplate->GetRegistrationName());
	}
	else
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::NativeDestruct] Unable to unregister tab correctly as no template is pinned. (%s)"), *GetName())
	}

	UnpinTemplate();
	

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
void UNEditorUtilityWidget::DelayedConstructTask()
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
	
	if (PinnedTemplate != nullptr)
	{
		UpdateEditorTab(PinnedTemplate->GetRegistrationName());
	}
	else
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::DelayedConstructTask] Unable to update tab details as no template is pinned. (%s)"), *GetName())
	}

	// We need a render to happen so this can be updated
	UnitScale = GetTickSpaceGeometry().GetAbsoluteSize() / GetTickSpaceGeometry().GetLocalSize();
}

void UNEditorUtilityWidget::UpdateEditorTab(const FName& InRegisteredName) const
{
	// TODO: Not updating anymore
	NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::UpdateEditorTab] Updating tab details for %s"), *InRegisteredName.ToString())
	
	if (const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(InRegisteredName))
	{
		Tab.Get()->SetTabIcon(GetTabDisplayIcon());
		Tab.Get()->SetLabel(GetTabDisplayText());
	}
}
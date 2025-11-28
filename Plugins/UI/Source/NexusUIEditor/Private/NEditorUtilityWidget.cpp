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

TArray<UNEditorUtilityWidget*> UNEditorUtilityWidget:: KnownEditorUtilityWidgets;

UNEditorUtilityWidget* UNEditorUtilityWidget::CreateFromWidget(TSubclassOf<UUserWidget> ContentWidget)
{
	const FString TemplatePath = FString::Printf(TEXT("/Script/Blutility.EditorUtilityWidgetBlueprint'/NexusUI/EditorResources/EUW_NWidgetTemplate.EUW_NWidgetTemplate'"));
	UBlueprint* TemplateWidget = LoadObject<UBlueprint>(nullptr, TemplatePath);
	if (TemplateWidget == nullptr)
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::CreateFromWidget] Unable to load template blueprint. (%s)"), *TemplatePath)
	}
	
	// TODO: We might need to duplicate the template object? as it seems to add to it...need a second window to test
	if (UEditorUtilityWidgetBlueprint* EditorWidget = Cast<UEditorUtilityWidgetBlueprint>(TemplateWidget))
	{
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		UEditorUtilityWidget* Widget = EditorUtilitySubsystem->SpawnAndRegisterTab(EditorWidget);

		if (UNEditorUtilityWidget* UtilityWidget = Cast<UNEditorUtilityWidget>(Widget); 
			UtilityWidget != nullptr)
		{
			UtilityWidget->PinTemplate(EditorWidget);
			if (ContentWidget != nullptr)
			{
				// Create widget
				UtilityWidget->BaseWidget = UtilityWidget->WidgetTree->ConstructWidget(ContentWidget,  MakeUniqueObjectName(UtilityWidget, ContentWidget, FName(TEXT("EUWContentWidget"))));
				
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

UE_DISABLE_OPTIMIZATION
bool UNEditorUtilityWidget::HasEditorUtilityWidgetOf(TSubclassOf<UNEditorUtilityWidget> WidgetClass)
{
	for (const auto KnownWidget : KnownEditorUtilityWidgets)
	{
		// TODO: Need to figure out hwo to tell if it is the one we want
		
	}
	return false;
}
UE_ENABLE_OPTIMIZATION

void UNEditorUtilityWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	KnownEditorUtilityWidgets.Add(this);

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
	
	KnownEditorUtilityWidgets.Remove(this);
	
	Super::NativeDestruct();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
void UNEditorUtilityWidget::DelayedConstructTask()
{
	// Will do its own checks
	//BuildContentFromWidget();
	
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
	if (const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(InRegisteredName))
	{
		Tab.Get()->SetTabIcon(GetTabDisplayIcon());
		Tab.Get()->SetLabel(GetTabDisplayText());
	}
}
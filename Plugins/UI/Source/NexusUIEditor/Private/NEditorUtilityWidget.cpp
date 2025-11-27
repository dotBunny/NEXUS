// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorUtilityWidget.h"

#include "EditorUtilityLibrary.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "NCoreEditorMinimal.h"

// THIS IS MEANT TO make future template based EUW's possible?
void UNEditorUtilityWidget::CreateFromWidget(UClass* WidgetClass)
{
	const FString TemplatePath = FString::Printf(TEXT("/Script/Blutility.EditorUtilityWidgetBlueprint'/NexusUI/EditorResources/EUW_NWidgetTemplate.EUW_NWidgetTemplate'"));
	UBlueprint* TemplateWidget = LoadObject<UBlueprint>(nullptr, TemplatePath);
	if (TemplateWidget == nullptr)
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::CreateFromWidget] Unable to load template blueprint. (%s)"), *TemplatePath)
	}
	
	if (UEditorUtilityWidgetBlueprint* EditorWidget = Cast<UEditorUtilityWidgetBlueprint>(TemplateWidget))
	{
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		UEditorUtilityWidget* Widget = EditorUtilitySubsystem->SpawnAndRegisterTab(EditorWidget);
		UNEditorUtilityWidget* UtilityWidget = Cast<UNEditorUtilityWidget>(Widget);
		
		UtilityWidget->BaseWidgetClass = WidgetClass;
		UtilityWidget->PinTemplate(EditorWidget);
	}
	else
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::CreateFromWidget] Template is not a UEditorUtilityWidgetBlueprint. (%s)"), *TemplatePath)
	}
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
	
	Super::NativeDestruct();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
void UNEditorUtilityWidget::DelayedConstructTask()
{
	if (PinnedTemplate != nullptr)
	{
		UpdateEditorTab(PinnedTemplate->GetRegistrationName());
	}
	else
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::DelayedConstructTask] Unable to update tab details as no template is pinned. (%s)"), *GetName())
	}

	if (BaseWidgetClass != nullptr)
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::NativeOnInitialized] Attempting to add base widget to EUW"), *BaseWidgetClass->GetName())
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
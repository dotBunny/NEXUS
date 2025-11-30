// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorUtilityWidget.h"

#include "EditorUtilityLibrary.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "NCoreEditorMinimal.h"

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

	// We need a render to happen so this can be updated
	UnitScale = GetTickSpaceGeometry().GetAbsoluteSize() / GetTickSpaceGeometry().GetLocalSize();
}

UE_DISABLE_OPTIMIZATION
void UNEditorUtilityWidget::UpdateEditorTab(const FName& InRegisteredName) const
{
	
	if (const TSharedPtr<SDockTab> ActiveTab = FGlobalTabmanager::Get()->FindExistingLiveTab(InRegisteredName))
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::UpdateEditorTab] Updating Active tab. (%s)"), *InRegisteredName.ToString())
		
		ActiveTab.Get()->SetTabIcon(GetTabDisplayIcon());
		ActiveTab.Get()->SetLabel(GetTabDisplayText());
		return;
	}
	
	
	FString NonActiveName = InRegisteredName.ToString();
	NonActiveName = NonActiveName.Replace(TEXT("_ActiveTab"), TEXT(" "));
	
	const TSharedRef<FGlobalTabmanager>& TabManager = FGlobalTabmanager::Get();
	
	if (const TSharedPtr<SDockTab> NonActiveTab = FGlobalTabmanager::Get()->FindExistingLiveTab(FName(NonActiveName)))
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::UpdateEditorTab] Updating Non-Active tab. (%s)"), *NonActiveName)
		NonActiveTab.Get()->SetTabIcon(GetTabDisplayIcon());
		NonActiveTab.Get()->SetLabel(GetTabDisplayText());
		return;
	}
	
	NE_LOG(Warning, TEXT("[UNEditorUtilityWidget::UpdateEditorTab] Unable to update tab details as tab does not exist. (%s)"), *InRegisteredName.ToString())
}
UE_ENABLE_OPTIMIZATION
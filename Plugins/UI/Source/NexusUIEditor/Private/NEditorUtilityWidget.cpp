// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorUtilityWidget.h"

#include "EditorUtilityLibrary.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "NEditorUtilityWidgetSystem.h"
#include "NEditorUtils.h"

const FString UNEditorUtilityWidget::WidgetState_WidgetBlueprint = TEXT("NWidget_WidgetBlueprint");
const FString UNEditorUtilityWidget::WidgetState_TabDisplayText = TEXT("NWidget_TabDisplayText");
const FString UNEditorUtilityWidget::WidgetState_TabIconStyle = TEXT("NWidget_TabIconStyle");
const FString UNEditorUtilityWidget::WidgetState_TabIconName = TEXT("NWidget_TabIconName");


void UNEditorUtilityWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	UNEditorUtilityWidgetSystem::RegisterWidget(this);
	
	// Bind our default behaviour
	OnTabClosedCallback.BindUObject(this, &UNEditorUtilityWidget::OnTabClosed);
	
	// We need to ensure that the window has its icon after all -- this oddly only executes once if you are opening multiple windows at once.
	UAsyncEditorDelay* DelayedConstructTask = NewObject<UAsyncEditorDelay>();
	DelayedConstructTask->Complete.AddDynamic(this, &UNEditorUtilityWidget::DelayedConstructTask);
	DelayedConstructTask->Start(0.01f, 1);
}

void UNEditorUtilityWidget::NativeDestruct()
{
	UNEditorUtilityWidgetSystem::UnregisterWidget(this);
	
	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	EditorUtilitySubsystem->UnregisterTabByID(GetTabIdentifier());
	
	UnpinTemplate();

	Super::NativeDestruct();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
void UNEditorUtilityWidget::DelayedConstructTask()
{
	FNEditorUtils::UpdateTab(GetTabIdentifier(), GetTabDisplayBrush(), GetTabDisplayText(), OnTabClosedCallback);
	FNEditorUtils::UpdateWorkspaceItem(GetTabIdentifier(), GetTabDisplayText(), GetTabDisplayIcon());
	
	// We need to do this _late_ as the identifier might not be set yet (as it could be based off the pinned template), unless overridden.
	if (IsPersistent())
	{
		UNEditorUtilityWidgetSystem* System = GEditor->GetEditorSubsystem<UNEditorUtilityWidgetSystem>();
		if (System->HasWidgetState(GetUserSettingsIdentifier()))
		{
			System->UpdateWidgetState(GetUserSettingsIdentifier(), GetState());
		}
		else
		{
			System->AddWidgetState(GetUserSettingsIdentifier(), GetUserSettingsTemplate(), GetState());
		}
	}
	
	// We need a render to happen so this can be updated
	UnitScale = GetTickSpaceGeometry().GetAbsoluteSize() / GetTickSpaceGeometry().GetLocalSize();
}

void UNEditorUtilityWidget::OnTabClosed(TSharedRef<SDockTab> Tab)
{
	if (IsPersistent() && !IsEngineExitRequested())
	{
		GEditor->GetEditorSubsystem<UNEditorUtilityWidgetSystem>()->RemoveWidgetState(GetUserSettingsIdentifier());
	}
}

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
	UE_LOG(LogNexusUIEditor, Warning, TEXT("UNEditorUtilityWidget::NativeConstruct: %s"), *GetUserSettingsIdentifier().ToString());
	Super::NativeConstruct();
	
	UE_LOG(LogNexusUIEditor, Warning, TEXT("REGISTER WIDGET"));
	UNEditorUtilityWidgetSystem::RegisterWidget(this);
	
	UE_LOG(LogNexusUIEditor, Warning, TEXT("BIND CLOSE"));
	// Bind our default behaviour
	if (OnTabClosedCallback.IsBound())
	{
		OnTabClosedCallback.Unbind();
	}
	OnTabClosedCallback.BindUObject(this, &UNEditorUtilityWidget::OnTabClosed);
	
	if (UNEditorUtilityWidgetSystem::IsRebuildingWidgets())
	{
		UNEditorUtilityWidgetSystem* System = GEditor->GetEditorSubsystem<UNEditorUtilityWidgetSystem>();
		System->RestoreTransientWidget(this);
		UE_LOG(LogNexusUIEditor, Warning, TEXT("REBUILDING WIDGET"));
	}
	
	// We need to ensure that the window has its icon after all -- this oddly only executes once if you are opening multiple windows at once.
	UE_LOG(LogNexusUIEditor, Warning, TEXT("MAKE TASK"));
	UAsyncEditorDelay* DelayedConstructTask = NewObject<UAsyncEditorDelay>();
	DelayedConstructTask->Complete.AddDynamic(this, &UNEditorUtilityWidget::DelayedConstructTask);
	UE_LOG(LogNexusUIEditor, Warning, TEXT("START TASK"));
	DelayedConstructTask->Start(0.01f, 1);
}

void UNEditorUtilityWidget::NativeDestruct()
{
	UE_LOG(LogNexusUIEditor, Warning, TEXT("UNEditorUtilityWidget::NativeDestruct: %s"), *GetUserSettingsIdentifier().ToString());
	UNEditorUtilityWidgetSystem::UnregisterWidget(this);
	
	// This stops it from transitioning between maps
	if (!IsPersistent())
	{
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		EditorUtilitySubsystem->UnregisterTabByID(GetTabIdentifier());
	}
	
	UnpinTemplate();

	Super::NativeDestruct();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
void UNEditorUtilityWidget::DelayedConstructTask()
{
	//PreDelayedConstructTask();
	UE_LOG(LogNexusUIEditor, Warning, TEXT("UNEditorUtilityWidget::DelayedConstructTask: %s"), *GetUserSettingsIdentifier().ToString());
	
	// We need to do this _late_ as the identifier might not be set yet (as it could be based off the pinned template), unless overridden.
	if (IsPersistent())
	{
		UNEditorUtilityWidgetSystem* System = GEditor->GetEditorSubsystem<UNEditorUtilityWidgetSystem>();
		if (System != nullptr)
		{
			if (System->HasWidgetState(GetUserSettingsIdentifier()))
			{
				UE_LOG(LogNexusUIEditor, Warning, TEXT("UNEditorUtilityWidget::DelayedConstructTask - HAS STATE"));
				RestoreWidgetState(this, GetUserSettingsIdentifier(), System->GetWidgetState(GetUserSettingsIdentifier()));
			}
			else
			{
				UE_LOG(LogNexusUIEditor, Warning, TEXT("UNEditorUtilityWidget::DelayedConstructTask - NEW STATE"));
				// By this time we have added a proper identifier through restore state
				System->AddWidgetState(GetUserSettingsIdentifier(), GetUserSettingsTemplate(), GetState());
			}
		}
	
	}
	
	UE_LOG(LogNexusUIEditor, Warning, TEXT("UNEditorUtilityWidget::DelayedConstructTask (POST): %s"), *GetUserSettingsIdentifier().ToString());
	
	
	FNEditorUtils::UpdateTab(GetTabIdentifier(), GetTabDisplayBrush(), GetTabDisplayText(), OnTabClosedCallback);
	FNEditorUtils::UpdateWorkspaceItem(GetTabIdentifier(), GetTabDisplayText(), GetTabDisplayIcon());
	
	
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

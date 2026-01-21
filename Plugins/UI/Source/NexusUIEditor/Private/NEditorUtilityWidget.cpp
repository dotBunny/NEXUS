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

void UNEditorUtilityWidget::SetTemplate(TObjectPtr<UEditorUtilityWidgetBlueprint> Template)
{
	PinnedTemplate = Template;
}

bool UNEditorUtilityWidget::IsPersistent() const
{
	return bIsPersistent;
}

FName UNEditorUtilityWidget::GetUserSettingsIdentifier()
{
	if (PinnedTemplate != nullptr)
    {
    	return PinnedTemplate->GetRegistrationName();
    }
    return GetFName();
}

FString UNEditorUtilityWidget::GetUserSettingsTemplate()
{
	if (PinnedTemplate != nullptr)
	{
		return PinnedTemplate->GetFullName();
	}
	return GetFullName();
}

void UNEditorUtilityWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Bind our default behaviour
	if (OnTabClosedCallback.IsBound())
	{
		OnTabClosedCallback.Unbind();
	}
	OnTabClosedCallback.BindUObject(this, &UNEditorUtilityWidget::OnTabClosed);
	
	if (IsPersistent())
	{
		UNEditorUtilityWidgetSystem* System = UNEditorUtilityWidgetSystem::Get();
		System->RegisterPersistentWidget(this);
		if (UNEditorUtilityWidgetSystem::IsRebuildingWidgets())
		{
			System->RestorePersistentWidget(this);
		}
	}
	
	// We need to ensure that the window has its icon after all -- this oddly only executes once if you are opening multiple windows at once.
	UAsyncEditorDelay* DelayedConstructTask = NewObject<UAsyncEditorDelay>();
	DelayedConstructTask->Complete.AddDynamic(this, &UNEditorUtilityWidget::DelayedConstructTask);
	DelayedConstructTask->Start(0.01f, 1);
}

void UNEditorUtilityWidget::NativeDestruct()
{
	if (IsPersistent())
	{
		UNEditorUtilityWidgetSystem::Get()->UnregisterPersistentWidget(this);
	}
	else
	{
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		EditorUtilitySubsystem->UnregisterTabByID(GetTabIdentifier());
	}
	Super::NativeDestruct();
}

FName UNEditorUtilityWidget::GetTabIdentifier()
{
	if (PinnedTemplate != nullptr)
	{
		return PinnedTemplate->GetRegistrationName();
	}
	return GetFName();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
void UNEditorUtilityWidget::DelayedConstructTask()
{
	// We need to do this _late_ as the identifier might not be set yet (as it could be based off the pinned template), unless overridden.
	if (IsPersistent())
	{
		UNEditorUtilityWidgetSystem* System = UNEditorUtilityWidgetSystem::Get();
		if (System != nullptr)
		{
			if (System->HasWidgetState(GetUserSettingsIdentifier()))
			{
				RestoreWidgetState(this, GetUserSettingsIdentifier(), System->GetWidgetState(GetUserSettingsIdentifier()));
			}
			else
			{
				// By this time we have added a proper identifier through restore state
				System->AddWidgetState(GetUserSettingsIdentifier(), GetUserSettingsTemplate(), GetWidgetState(this));
			}
		}
	}
	
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

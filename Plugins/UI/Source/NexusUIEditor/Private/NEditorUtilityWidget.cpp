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

TSharedPtr<SDockTab> UNEditorUtilityWidget::GetTab()
{
	const TSharedPtr<SWidget> BaseWidget = this->TakeWidget();
	TSharedPtr<SWidget> Widget = BaseWidget;
	while (Widget.IsValid())
	{
		// Bound Tab
		if (Widget->GetType() == FName("SDockingTabStack"))
		{
			FChildren* Children = Widget->GetChildren();
			int ChildrenCount = Children->Num();
			
			for (int i = 0; i < ChildrenCount; ++i)
			{

				const TSharedPtr<SWidget> ChildWidget = Children->GetChildAt(i);
				TSharedPtr<SWidget> Found = FNEditorUtils::FindWidgetByType(ChildWidget, FName("SDockTab"));
				if (Found.IsValid())
				{
					return StaticCastSharedPtr<SDockTab>(Found);
				}
			}
		}
		
		// Floating Tab  ?
		if (Widget->GetType() == FName("SDockTab"))
		{
			return StaticCastSharedPtr<SDockTab>(Widget);
		}
		
		Widget = Widget->GetParentWidget();
	}
	return nullptr;
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
	DelayedConstructTask->RegisterWithGameInstance(nullptr);
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
		const TSharedPtr<SDockTab> Tab = GetTab();
		if (Tab.IsValid())
		{
			EditorUtilitySubsystem->UnregisterTabByID( FName(Tab->GetLayoutIdentifier().ToString()));
		}
	}
	Super::NativeDestruct();
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
	
	
	
	TSharedPtr<SDockTab> Tab = GetTab();
	if (Tab.IsValid())
	{
		if (GetTabDisplayBrush().IsSet())
		{
			Tab->SetTabIcon(GetTabDisplayBrush());
		}
		
		if (!GetTabDisplayText().IsEmpty())
		{
			Tab->SetLabel(GetTabDisplayText());
		}
		
		if (OnTabClosedCallback.IsBound())
		{
			Tab->SetOnTabClosed(OnTabClosedCallback);
		}

		FNEditorUtils::UpdateWorkspaceItem(Tab->GetLayoutIdentifier().TabType, GetTabDisplayText(), GetTabDisplayIcon());
	}
	else
	{
		UE_LOG(LogNexusUIEditor, Warning, TEXT("Unable to update SDockTab as it could not be found."))
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

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorUtilityWidget.h"

#include "EditorUtilityLibrary.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "NEditorUtilityWidgetSubsystem.h"
#include "NEditorUtils.h"
#include "NSlateUtils.h"

UEditorUtilityWidget* UNEditorUtilityWidget::SpawnTab(const FString& ObjectPath, FName Identifier)
{
	if (Identifier != NAME_None)
	{
		UNEditorUtilityWidgetSubsystem* System = UNEditorUtilityWidgetSubsystem::Get();
		if (System != nullptr && System->HasWidget(Identifier))
		{
			UNEditorUtilityWidget* Widget = System->GetWidget(Identifier);
			if (Widget != nullptr)
			{
				const TSharedPtr<SDockTab> Tab = FNSlateUtils::FindDocTabWithLabel(
					Widget->TakeWidget(), 
					Widget->GetTabDisplayName(),
					Widget->GetTabIdentifier());
				if (Tab.IsValid())
				{
					Tab->FlashTab();
				}
			}
			return System->GetWidget(Identifier);
		}
	}
	
	if(UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>())
	{
		const TObjectPtr<UEditorUtilityWidgetBlueprint> LoadedWidgetBlueprint = 
			LoadObject<UEditorUtilityWidgetBlueprint>(GetTransientPackage(), ObjectPath);
		
		// Use an internal system to spawn and register the widget
		FName TabIdentifier;
		UEditorUtilityWidget* SpawnedWidget = EditorUtilitySubsystem->SpawnAndRegisterTabAndGetID(LoadedWidgetBlueprint, TabIdentifier);
		UNEditorUtilityWidget* Widget = Cast<UNEditorUtilityWidget>(SpawnedWidget);
		if (Widget)
		{
			Widget->CachedTabIdentifier = TabIdentifier;
		}
		return SpawnedWidget;
	}
	return nullptr;
}


void UNEditorUtilityWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Ensure that we have some sort of identifier
	if (StateIdentifier == NAME_None)
	{
		StateIdentifier = FName(GetFName());
	}
	
	// Bind our default behaviour
	if (OnTabClosedCallback.IsBound())
	{
		OnTabClosedCallback.Unbind();
	}
	OnTabClosedCallback.BindUObject(this, &UNEditorUtilityWidget::OnTabClosed);
	
	UNEditorUtilityWidgetSubsystem* System = UNEditorUtilityWidgetSubsystem::Get();
	if (System != nullptr)
	{
		System->RegisterWidget(this);
	}
	
	// If we have icon data set we should create the icon
	if (TabIconStyle != NAME_None && TabIconName != NAME_None)
	{
		TabIcon = FSlateIcon(TabIconStyle, FName(TabIconName));
	}
	
	// We need to make a callback after the window has been constructed further to ensure we have a tab
	DelayedTask = NewObject<UAsyncEditorDelay>(this, NAME_None, RF_Transient);
	DelayedTask->RegisterWithGameInstance(this);
	DelayedTask->Complete.AddDynamic(this, &UNEditorUtilityWidget::DelayedConstructTask);
	DelayedTask->Start(0.01f, 1);
}

void UNEditorUtilityWidget::NativeDestruct()
{
	// Remove the delayed task to prevent dangling behaviour.
	if (DelayedTask->IsValidLowLevel())
	{
		DelayedTask->Complete.RemoveAll(this);
		DelayedTask = nullptr;
	}
	
	UNEditorUtilityWidgetSubsystem* System = UNEditorUtilityWidgetSubsystem::Get();
	if (System != nullptr)
	{
		System->UnregisterWidget(this);
	}
	
	Super::NativeDestruct();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
void UNEditorUtilityWidget::DelayedConstructTask()
{
	const TSharedPtr<SDockTab> Tab = FNSlateUtils::FindDocTabWithLabel(
		this->TakeWidget(), 
		GetTabDisplayName(),
		GetTabIdentifier());
	
	if (Tab.IsValid())
	{
		Tab->SetLabel(GetTabDisplayName());
		
		if (TabIcon.IsSet())
		{
			Tab->SetTabIcon(TabIcon.GetIcon());
		}
		
		// Bind tab closed callback
		if (OnTabClosedCallback.IsBound())
		{
			Tab->SetOnTabClosed(OnTabClosedCallback);
		}

		FNEditorUtils::UpdateWorkspaceItem(Tab->GetLayoutIdentifier().TabType, GetTabDisplayName(), TabIcon);
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
	if (IsPersistent() && !IsEngineExitRequested() && !bHasPermanentState)
	{
		GEditor->GetEditorSubsystem<UNEditorUtilityWidgetSubsystem>()->RemoveWidgetState(GetStateIdentifier());
	}
}

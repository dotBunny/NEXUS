// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWidgetEditorUtilityWidget.h"

#include "EditorUtilityWidgetComponents.h"
#include "NEditorUtilityWidgetSystem.h"
#include "NEditorUtils.h"
#include "NUIEditorMinimal.h"
#include "Blueprint/WidgetTree.h"

TMap<FName, UNWidgetEditorUtilityWidget*> UNWidgetEditorUtilityWidget:: KnownEditorUtilityWidgets;
const FString UNWidgetEditorUtilityWidget::TemplatePath = TEXT("/Script/Blutility.EditorUtilityWidgetBlueprint'/NexusUI/EditorResources/EUW_NWidgetWrapper.EUW_NWidgetWrapper'");

UNWidgetEditorUtilityWidget* UNWidgetEditorUtilityWidget::GetOrCreate(const FName Identifier, const FString& WidgetBlueprint, const FText& TabDisplayText,  const FName& TabIconStyle, const FString& TabIconName)
{
	if (Identifier == NAME_None)
	{
		UE_LOG(LogNexusUIEditor, Error, TEXT("A proper Identifier(NAME_None) must be provided for a UNWidgetEditorUtilityWidget to be created."))
		return nullptr;
	}
	
	// Return existing and flash it
	if (UNWidgetEditorUtilityWidget* ExistingWidget = GetEditorUtilityWidget(Identifier); 
		ExistingWidget != nullptr)
	{
		FNEditorUtils::FocusTab(ExistingWidget->GetTabIdentifier());
		return ExistingWidget;
	}
	

	FNWidgetState WidgetState = CreateWidgetState(WidgetBlueprint, TabDisplayText, TabIconStyle, TabIconName);
	UNEditorUtilityWidget* Widget = UNEditorUtilityWidgetSystem::CreateWithState(TemplatePath, Identifier, WidgetState);
	return Cast<UNWidgetEditorUtilityWidget>(Widget);
}

bool UNWidgetEditorUtilityWidget::HasEditorUtilityWidget(const FName Identifier)
{
	return KnownEditorUtilityWidgets.Contains(Identifier);
}

UNWidgetEditorUtilityWidget* UNWidgetEditorUtilityWidget::GetEditorUtilityWidget(const FName Identifier)
{
	if (KnownEditorUtilityWidgets.Contains(Identifier))
	{
		return KnownEditorUtilityWidgets[Identifier];
	}
	return nullptr;
}

FText UNWidgetEditorUtilityWidget::GetTabDisplayText() const
{
	if (BaseWidget != nullptr && BaseWidget->Implements<UNWidgetTabDetailsProvider>())
	{
		const INWidgetTabDetailsProvider* TabDetails = Cast<INWidgetTabDetailsProvider>(BaseWidget);
		return TabDetails->GetTabDisplayText();
	}
	return TabDisplayText;
}

FSlateIcon UNWidgetEditorUtilityWidget::GetTabDisplayIcon()
{
	if (BaseWidget != nullptr && BaseWidget->Implements<UNWidgetTabDetailsProvider>())
	{
		INWidgetTabDetailsProvider* TabDetails = Cast<INWidgetTabDetailsProvider>(BaseWidget);
		return TabDetails->GetTabDisplayIcon();
	}
		
	if (!TabIcon.IsSet() && !TabIconStyle.IsNone() && TabIconName.Len() > 0)
	{
		TabIcon = FSlateIcon(TabIconStyle, FName(TabIconName));
	}
	return TabIcon;
}

TAttribute<const FSlateBrush*> UNWidgetEditorUtilityWidget::GetTabDisplayBrush()
{
	if (BaseWidget != nullptr && BaseWidget->Implements<UNWidgetTabDetailsProvider>())
	{
		INWidgetTabDetailsProvider* TabDetails = Cast<INWidgetTabDetailsProvider>(BaseWidget);
		return TabDetails->GetTabDisplayBrush();
	}
		
	if (!TabIcon.IsSet() && !TabIconStyle.IsNone() && TabIconName.Len() > 0)
	{
		TabIcon = FSlateIcon(TabIconStyle, FName(TabIconName));
	}
	if (TabIcon.IsSet())
	{
		return TabIcon.GetIcon();
	}
	return TAttribute<const FSlateBrush*>();
}

FNWidgetState UNWidgetEditorUtilityWidget::GetWidgetState(UObject* BlueprintWidget)
{
	FNWidgetState BaseState = CreateWidgetState(WidgetBlueprint, TabDisplayText, TabIconStyle, TabIconName);
	FNWidgetState BlueprintState = Execute_OnGetWidgetStateEvent(BlueprintWidget);
	BaseState.OverlayState(BlueprintState, false);
		
	// Add in child data that WILL NOT override parent keys
	if (BaseWidget != nullptr && BaseWidget->Implements<UNWidgetStateProvider>())
	{
		FNWidgetState ChildWidgetState;
		if (INWidgetStateProvider* StateProvider = Cast<INWidgetStateProvider>(BaseWidget); 
			StateProvider != nullptr)
		{
			ChildWidgetState = StateProvider->GetWidgetState(BaseWidget);
		}
		else
		{
			ChildWidgetState = Execute_OnGetWidgetStateEvent(BaseWidget);
		}
		BaseState.OverlayState(ChildWidgetState, false);
	}
	
	return BaseState;
}

FNWidgetState UNWidgetEditorUtilityWidget::CreateWidgetState(const FString& WidgetBlueprint,
	const FText& TabDisplayText, const FName& TabIconStyle, const FString& TabIconName)
{
	FNWidgetState WidgetState;
		
	WidgetState.AddString(WidgetState_WidgetBlueprint, WidgetBlueprint);
	WidgetState.AddString(WidgetState_TabDisplayText, TabDisplayText.ToString());
	WidgetState.AddString(WidgetState_TabIconStyle, TabIconStyle.ToString());
	WidgetState.AddString(WidgetState_TabIconName, TabIconName);
		
	return WidgetState;
}

void UNWidgetEditorUtilityWidget::RestoreWidgetState(UObject* BlueprintWidget, FName Identifier, FNWidgetState& WidgetState)
{
	// Cache all our state objects
	WidgetIdentifier = Identifier;
	TabDisplayText = FText::FromString(WidgetState.GetString(WidgetState_TabDisplayText));
	TabIconStyle = FName(WidgetState.GetString(WidgetState_TabIconStyle));
	TabIconName = WidgetState.GetString(WidgetState_TabIconName);
	Execute_OnRestoreWidgetStateEvent(BlueprintWidget, Identifier, WidgetState);
	
	FString NewWidgetBlueprint = WidgetState.GetString(WidgetState_WidgetBlueprint);
	if (WidgetBlueprint != NewWidgetBlueprint)
	{
		// We already have a base widget, but this is changing it for some odd reason?
		if (BaseWidget != nullptr)
		{
			UEditorUtilityScrollBox* EditorScrollBox = Cast<UEditorUtilityScrollBox>(WidgetTree->RootWidget);
			EditorScrollBox->RemoveChild(BaseWidget);
			WidgetTree->RemoveWidget(BaseWidget);
			BaseWidget = nullptr;
			WidgetBlueprint = TEXT("");
		}
		
		const UWidgetBlueprint* ContentWidgetTemplate = LoadObject<UWidgetBlueprint>(nullptr, NewWidgetBlueprint);
		const TSubclassOf<UUserWidget> ContentWidget{ContentWidgetTemplate->GeneratedClass};
		if (ContentWidget != nullptr)
		{
			BaseWidget = WidgetTree->ConstructWidget(ContentWidget);
			WidgetBlueprint = NewWidgetBlueprint;
			
			UEditorUtilityScrollBox* EditorScrollBox = Cast<UEditorUtilityScrollBox>(WidgetTree->RootWidget);
			EditorScrollBox->AddChild(BaseWidget);
			
			// Pass WidgetState Along
			if (BaseWidget->Implements<UNWidgetStateProvider>())
			{
				if (INWidgetStateProvider* StateProvider = Cast<INWidgetStateProvider>(BaseWidget); 
					StateProvider != nullptr)
				{
					StateProvider->RestoreWidgetState(BaseWidget, Identifier, WidgetState);
				}
				else
				{
					Execute_OnRestoreWidgetStateEvent(BaseWidget, Identifier, WidgetState);
				}
			}
		}
		else
		{
			UE_LOG(LogNexusUIEditor, Warning, TEXT("Unable to find content widget(%s) to use with the UNWidgetEditorUtilityWidget."), *TemplatePath)
		}
	}
}

void UNWidgetEditorUtilityWidget::NativeDestruct()
{
	KnownEditorUtilityWidgets.Remove(WidgetIdentifier);
	Super::NativeDestruct();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
void UNWidgetEditorUtilityWidget::DelayedConstructTask()
{
	// We register a frame later as the base widget data has been set at that point
	KnownEditorUtilityWidgets.Add(WidgetIdentifier, this);
	Super::DelayedConstructTask();
}
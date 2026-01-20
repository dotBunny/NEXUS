// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWidgetEditorUtilityWidget.h"

#include "EditorUtilityWidgetComponents.h"
#include "NEditorUtilityWidgetSystem.h"
#include "NEditorUtils.h"
#include "NUIEditorMinimal.h"
#include "Blueprint/WidgetTree.h"

const FString UNWidgetEditorUtilityWidget::TemplatePath = TEXT("/Script/Blutility.EditorUtilityWidgetBlueprint'/NexusUI/EditorResources/EUW_NWidgetWrapper.EUW_NWidgetWrapper'");


TMap<FName, UNWidgetEditorUtilityWidget*> UNWidgetEditorUtilityWidget:: KnownWidgets;
TMap<FString, TSharedPtr<UWidgetBlueprint>> UNWidgetEditorUtilityWidget::WidgetBlueprints;

TSharedPtr<UWidgetBlueprint> UNWidgetEditorUtilityWidget::GetSharedWidgetBlueprint(const FString& InTemplate)
{
	if (WidgetBlueprints.Contains(InTemplate))
	{
		return WidgetBlueprints[InTemplate].ToSharedRef();
	}
	// DO we need to force compile?
	// FKismetEditorUtilities::CompileBlueprint(UBlueprint* BlueprintObj, EBlueprintCompileOptions CompileFlags, FCompilerResultsLog* pResults)
	
	UWidgetBlueprint* NewWidget = LoadObject<UWidgetBlueprint>(UNEditorUtilityWidgetSystem::Get(), InTemplate);
	if (NewWidget == nullptr)
	{
		UE_LOG(LogNexusUIEditor, Error, TEXT("Unable to create a UNWidgetEditorUtilityWidget as the provided UWidgetBlueprint(%s) was unable to load."), *InTemplate)
		return nullptr;
	}
	NewWidget->SetFlags(RF_Public | RF_Transient | RF_DuplicateTransient);
	
	WidgetBlueprints.Add(InTemplate, MakeShareable<UWidgetBlueprint>(NewWidget));
	
	return WidgetBlueprints[InTemplate].ToSharedRef();
}


UNWidgetEditorUtilityWidget* UNWidgetEditorUtilityWidget::GetOrCreate(const FName Identifier, const FString& WidgetBlueprint, const FText& TabDisplayText,  const FName& TabIconStyle, const FString& TabIconName)
{
	if (Identifier == NAME_None)
	{
		UE_LOG(LogNexusUIEditor, Error, TEXT("A proper Identifier(NAME_None) must be provided for a UNWidgetEditorUtilityWidget to be created."))
		return nullptr;
	}
	
	// Return existing and flash it
	UNWidgetEditorUtilityWidget* ExistingWidget = GetWidget(Identifier); 
	if (ExistingWidget != nullptr)
	{
		FNEditorUtils::FocusTab(ExistingWidget->GetTabIdentifier());
		return ExistingWidget;
	}
	

	FNWidgetState WidgetState = CreateWidgetState(WidgetBlueprint, TabDisplayText, TabIconStyle, TabIconName);
	UEditorUtilityWidget* Widget = UNEditorUtilityWidgetSystem::CreateWithState(TemplatePath, Identifier, WidgetState);
	return Cast<UNWidgetEditorUtilityWidget>(Widget);
}

bool UNWidgetEditorUtilityWidget::HasWidget(const FName Identifier)
{
	return KnownWidgets.Contains(Identifier);
}

UNWidgetEditorUtilityWidget* UNWidgetEditorUtilityWidget::GetWidget(const FName Identifier)
{
	if (KnownWidgets.Contains(Identifier))
	{
		return KnownWidgets[Identifier];
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
		BaseState.DumpToLog();
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
	if (WidgetBlueprint == NewWidgetBlueprint) return;

	// We already have a base widget, but this is changing it for some odd reason?
	if (BaseWidget != nullptr)
	{
		UEditorUtilityScrollBox* EditorScrollBox = Cast<UEditorUtilityScrollBox>(WidgetTree->RootWidget);
		EditorScrollBox->RemoveChild(BaseWidget);
		WidgetTree->RemoveWidget(BaseWidget);
		BaseWidget = nullptr;
		WidgetBlueprint = TEXT("");
	}
	
	// Manage our spawned widgets were loading as they can be reused during different scenarios
	// V2 ContentWidgetTemplate = GetSharedWidgetBlueprint(NewWidgetBlueprint);
	
	
	// V1 modified to SharedPtr
	ContentWidgetTemplate = LoadObject<UWidgetBlueprint>(this, NewWidgetBlueprint);
	if (ContentWidgetTemplate == nullptr)
	{
		UE_LOG(LogNexusUIEditor, Error, TEXT("Unable to create a UNWidgetEditorUtilityWidget as the provided UWidgetBlueprint(%s) was unable to load."), *NewWidgetBlueprint)
		return;
	}
	ContentWidgetTemplate->SetFlags(RF_Public | RF_Transient | RF_DuplicateTransient);
	
	
	
	const TSubclassOf<UUserWidget> ContentWidget
	{
		ContentWidgetTemplate->GeneratedClass
	};
	
	if (ContentWidget != nullptr)
 	{
		// The widget tree might be the generated class thats nulled?
		BaseWidget = WidgetTree->ConstructWidget(ContentWidget);
		WidgetBlueprint = NewWidgetBlueprint;
		
		UEditorUtilityScrollBox* EditorScrollBox = Cast<UEditorUtilityScrollBox>(WidgetTree->RootWidget);
		EditorScrollBox->AddChild(BaseWidget);
		
		// Pass WidgetState Along
		InvokeRestoreWidgetState(BaseWidget, Identifier, WidgetState);
	}
	else
	{
		UE_LOG(LogNexusUIEditor, Warning, TEXT("Unable to find content widget(%s) to use with the UNWidgetEditorUtilityWidget."), *TemplatePath)
	}
	
	SetWidgetStateHasBeenRestored(true);
}

void UNWidgetEditorUtilityWidget::ResetWidgetBlueprints()
{
	for (auto& Blueprint : WidgetBlueprints)
	{
		Blueprint.Value.Reset();
	}
	WidgetBlueprints.Empty();
}

void UNWidgetEditorUtilityWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (IsPersistent() && !HasWidgetStateBeenRestored())
	{
		UNEditorUtilityWidgetSystem::RestorePersistentWidget(this);
	}
}


void UNWidgetEditorUtilityWidget::NativeDestruct()
{
	KnownWidgets.Remove(WidgetIdentifier);
	Super::NativeDestruct();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
void UNWidgetEditorUtilityWidget::DelayedConstructTask()
{
	// We register a frame later as the base widget data has been set at that point
	KnownWidgets.Add(WidgetIdentifier, this);
	Super::DelayedConstructTask();
}
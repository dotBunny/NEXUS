// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWidgetEditorUtilityWidget.h"

#include "INWidgetMessageProvider.h"
#include "NEditorUtilityWidgetSystem.h"
#include "NSlateUtils.h"
#include "NUIEditorMinimal.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"

const FString UNWidgetEditorUtilityWidget::TemplatePath = TEXT("/Script/Blutility.EditorUtilityWidgetBlueprint'/NexusUI/EditorResources/EUW_NWidgetWrapper.EUW_NWidgetWrapper'");


TMap<FName, UNWidgetEditorUtilityWidget*> UNWidgetEditorUtilityWidget:: KnownWidgets;

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
		const TSharedPtr<SDockTab> Tab = FNSlateUtils::FindDocTab(ExistingWidget->TakeWidget());
		if (Tab.IsValid())
		{
			Tab->ActivateInParent(SetDirectly);
			Tab->FlashTab();
		}

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
		FNWidgetState ChildWidgetState = INWidgetStateProvider::InvokeGetWidgetState(BaseWidget);
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
	if (WidgetBlueprint == NewWidgetBlueprint) return;

	// We already have a base widget, but this is changing it for some odd reason?
	if (BaseWidget != nullptr)
	{
		ContentPanel->RemoveChild(BaseWidget);
		WidgetTree->RemoveWidget(BaseWidget);
		
		N_UNBIND_WIDGET_MESSAGE_PROVIDED(BaseWidget, OnStatusProviderUpdateHandle);
		
		BaseWidget = nullptr;
		WidgetBlueprint = TEXT("");
	}
	
	ContentWidgetTemplate = LoadObject<UWidgetBlueprint>(this, NewWidgetBlueprint);
	if (ContentWidgetTemplate == nullptr)
	{
		UE_LOG(LogNexusUIEditor, Error, TEXT("Unable to create a UNWidgetEditorUtilityWidget as the provided UWidgetBlueprint(%s) was unable to load."), *NewWidgetBlueprint)
		return;
	}
	
	const TSubclassOf<UUserWidget> ContentWidget { ContentWidgetTemplate->GeneratedClass };
	
	if (ContentWidget != nullptr)
 	{
		// The widget tree might be the generated class that's nulled?
		BaseWidget = WidgetTree->ConstructWidget(ContentWidget);
		
		// Bind to updates of a status provider
		N_BIND_WIDGET_MESSAGE_PROVIDED(BaseWidget, OnStatusProviderUpdateHandle, UNWidgetEditorUtilityWidget::UpdateHeader);
		
		// Check for some known crash things
		if (BaseWidget->IsFocusable())
		{
			UE_LOG(LogNexusUIEditor, Error, TEXT("UNWidgetEditorUtilityWidgets cannot have focusable content (%s)."), *ContentWidget->GetName())
		}
		
		WidgetBlueprint = NewWidgetBlueprint;
		
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ContentPanel->AddChild(BaseWidget));
		if (CanvasSlot)
		{
			CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			CanvasSlot->SetOffsets(FMargin(0.0f));
		}
		
		
		
		// Pass WidgetState Along
		InvokeRestoreWidgetState(BaseWidget, Identifier, WidgetState);
		
		// Initial update of the header
		UpdateHeader();
	}
	else
	{
		UE_LOG(LogNexusUIEditor, Warning, TEXT("Unable to find content widget(%s) to use with the UNWidgetEditorUtilityWidget."), *TemplatePath)
	}
	
	SetWidgetStateHasBeenRestored(true);
}


void UNWidgetEditorUtilityWidget::UpdateHeader() const
{
	if (BaseWidget == nullptr || !INWidgetMessageProvider::InvokeHasStatusProviderMessage(BaseWidget))
	{
		Header->SetVisibility(ESlateVisibility::Collapsed);
		HeaderFooter->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	HeaderText->SetText(FText::FromString(INWidgetMessageProvider::InvokeGetStatusProviderMessage(BaseWidget)));
	Header->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	HeaderFooter->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UNWidgetEditorUtilityWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsPersistent())
	{
		OnBlueprintPreCompileHandle = GEditor->OnBlueprintPreCompile().AddUObject(this, &UNWidgetEditorUtilityWidget::OnBlueprintPreCompile);
		if (!HasWidgetStateBeenRestored())
		{
			UNEditorUtilityWidgetSystem::Get()->RestorePersistentWidget(this);
		}
	}

	UpdateHeader();
}


void UNWidgetEditorUtilityWidget::NativeDestruct()
{
	if (IsPersistent())
	{
		GEditor->OnBlueprintPreCompile().Remove(OnBlueprintPreCompileHandle);
	}
	KnownWidgets.Remove(WidgetIdentifier);
	Super::NativeDestruct();
}

void UNWidgetEditorUtilityWidget::OnBlueprintPreCompile(UBlueprint* Blueprint)
{
	if (Blueprint == PinnedTemplate || Blueprint == ContentWidgetTemplate)
	{
		UNEditorUtilityWidgetSystem::Get()->AddPersistentState(this);
	}
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
void UNWidgetEditorUtilityWidget::DelayedConstructTask()
{
	// We register a frame later as the base widget data has been set at that point
	KnownWidgets.Add(WidgetIdentifier, this);
	Super::DelayedConstructTask();
}
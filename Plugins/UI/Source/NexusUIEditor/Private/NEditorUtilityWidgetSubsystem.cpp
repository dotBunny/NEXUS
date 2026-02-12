#include "NEditorUtilityWidgetSubsystem.h"
#include "NEditorUtilityWidget.h"



void UNEditorUtilityWidgetSubsystem::AddWidgetState(const FName& Identifier, const FNWidgetState& WidgetState)
{
	if (WidgetStates.AddWidgetState(Identifier, WidgetState))
	{
		SaveConfig();
	}
}

void UNEditorUtilityWidgetSubsystem::RemoveWidgetState(const FName& Identifier)
{
	if (WidgetStates.RemoveWidgetState(Identifier))
	{
		SaveConfig();
	}
}

void UNEditorUtilityWidgetSubsystem::UpdateWidgetState(const FName& Identifier, const FNWidgetState& WidgetState)
{
	if (WidgetStates.UpdateWidgetState(Identifier, WidgetState))
	{
		SaveConfig();
	}
}

bool UNEditorUtilityWidgetSubsystem::HasWidgetState(const FName& Identifier) const
{
	return WidgetStates.HasWidgetState(Identifier);
}

FNWidgetState& UNEditorUtilityWidgetSubsystem::GetWidgetState(const FName& Identifier)
{
	return WidgetStates.GetWidgetState(Identifier);
}

void UNEditorUtilityWidgetSubsystem::RegisterWidget(UNEditorUtilityWidget* Widget)
{
	if (!KnownWidgets.Contains(Widget->GetStateIdentifier()))
	{
		KnownWidgets.Add(Widget->GetStateIdentifier(), Widget);
	}
}

void UNEditorUtilityWidgetSubsystem::UnregisterWidget(const UNEditorUtilityWidget* Widget)
{
	if (KnownWidgets.Contains(Widget->GetStateIdentifier()))
	{
		KnownWidgets.Remove(Widget->GetStateIdentifier());
	}
}

UNEditorUtilityWidget* UNEditorUtilityWidgetSubsystem::GetWidget(const FName& Identifier)
{
	return KnownWidgets.FindRef(Identifier);
}

bool UNEditorUtilityWidgetSubsystem::HasWidget(const FName& Identifier)
{
	return KnownWidgets.Contains(Identifier);
}

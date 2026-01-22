#include "NEditorUtilityWidgetSystem.h"
#include "NEditorUtilityWidget.h"



void UNEditorUtilityWidgetSystem::AddWidgetState(const FName& Identifier, const FNWidgetState& WidgetState)
{
	if (WidgetStates.AddWidgetState(Identifier, WidgetState))
	{
		SaveConfig();
	}
}

void UNEditorUtilityWidgetSystem::RemoveWidgetState(const FName& Identifier)
{
	if (WidgetStates.RemoveWidgetState(Identifier))
	{
		SaveConfig();
	}
}

void UNEditorUtilityWidgetSystem::UpdateWidgetState(const FName& Identifier, const FNWidgetState& WidgetState)
{
	if (WidgetStates.UpdateWidgetState(Identifier, WidgetState))
	{
		SaveConfig();
	}
}

bool UNEditorUtilityWidgetSystem::HasWidgetState(const FName& Identifier) const
{
	return WidgetStates.HasWidgetState(Identifier);
}

FNWidgetState& UNEditorUtilityWidgetSystem::GetWidgetState(const FName& Identifier)
{
	return WidgetStates.GetWidgetState(Identifier);
}

void UNEditorUtilityWidgetSystem::RegisterWidget(UNEditorUtilityWidget* Widget)
{
	if (!KnownWidgets.Contains(Widget->GetStateIdentifier()))
	{
		KnownWidgets.Add(Widget->GetStateIdentifier(), Widget);
	}
}

void UNEditorUtilityWidgetSystem::UnregisterWidget(const UNEditorUtilityWidget* Widget)
{
	if (KnownWidgets.Contains(Widget->GetStateIdentifier()))
	{
		KnownWidgets.Remove(Widget->GetStateIdentifier());
	}
}

UNEditorUtilityWidget* UNEditorUtilityWidgetSystem::GetWidget(const FName& Identifier)
{
	return KnownWidgets.FindRef(Identifier);
}

bool UNEditorUtilityWidgetSystem::HasWidget(const FName& Identifier)
{
	return KnownWidgets.Contains(Identifier);
}

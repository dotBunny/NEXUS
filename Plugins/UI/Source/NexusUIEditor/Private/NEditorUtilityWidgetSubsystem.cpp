// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

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
	if (!KnownWidgets.Contains(Widget->GetUniqueIdentifier()))
	{
		KnownWidgets.Add(Widget->GetUniqueIdentifier(), Widget);
	}
}

void UNEditorUtilityWidgetSubsystem::UnregisterWidget(const UNEditorUtilityWidget* Widget)
{
	if (KnownWidgets.Contains(Widget->GetUniqueIdentifier()))
	{
		KnownWidgets.Remove(Widget->GetUniqueIdentifier());
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

FName UNEditorUtilityWidgetSubsystem::GetTabIdentifier(const FName WidgetIdentifier)
{
	for (int i = 0; i < WidgetTabIdentifiers.WidgetIdentifiers.Num(); i++)
	{
		if (WidgetTabIdentifiers.WidgetIdentifiers[i] == WidgetIdentifier)
		{
			return WidgetTabIdentifiers.TabIdentifier[i];
		}
	}
	return NAME_None;
}

void UNEditorUtilityWidgetSubsystem::SetTabIdentifier(const FName WidgetIdentifier, const FName TabIdentifier)
{
	for (int i = 0; i < WidgetTabIdentifiers.WidgetIdentifiers.Num(); i++)
	{
		if (WidgetTabIdentifiers.WidgetIdentifiers[i] == WidgetIdentifier)
		{
			WidgetTabIdentifiers.TabIdentifier[i] = TabIdentifier;
			SaveConfig();
			return;
		}
	}
	
	WidgetTabIdentifiers.WidgetIdentifiers.Add(WidgetIdentifier);
	WidgetTabIdentifiers.TabIdentifier.Add(TabIdentifier);
	SaveConfig();
}

void UNEditorUtilityWidgetSubsystem::RemoveTabIdentifier(const FName WidgetIdentifier)
{
	for (int i = WidgetTabIdentifiers.WidgetIdentifiers.Num() - 1; i >= 0; i--)
	{
		if (WidgetTabIdentifiers.WidgetIdentifiers[i] == WidgetIdentifier)
		{
			WidgetTabIdentifiers.WidgetIdentifiers.RemoveAt(i);
			WidgetTabIdentifiers.TabIdentifier.RemoveAt(i);
			SaveConfig();
		}
	}
}

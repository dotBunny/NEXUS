#include "NEditorUtilityWidgetUserSettings.h"

#include "NCoreEditorMinimal.h"

void UNEditorUtilityWidgetUserSettings::RegisterWidget(const FName& Identifier, const FString& Template, const FNEditorUtilityWidgetUserSettingsPayload& Payload)
{
	// Check for already registered
	if (int32 WorkingIndex = GetIdentifierIndex(Identifier); 
		WorkingIndex == INDEX_NONE)
	{
		// Add our items
		WorkingIndex = Identifiers.Add(Identifier);
		const int32 PayloadIndexCheck = Payloads.Add(Payload);
		const int32 TemplateIndexCheck = Templates.Add(Template);
		
		// Sanity check
		if (WorkingIndex != PayloadIndexCheck || WorkingIndex != TemplateIndexCheck)
		{
			NE_LOG(Error, TEXT("[UNEditorUtilityWidgetUserSettings::RegisterWidget] Index count invalid, clearing cache."))
			ClearCache();
		}
	}
	else
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidgetUserSettings::RegisterWidget] Widget with identifier %s already registered, updating payload only."), *Identifier.ToString());
		Payloads[WorkingIndex] = Payload;
	}
	
	SaveConfig();

}

void UNEditorUtilityWidgetUserSettings::UnregisterWidget(const FName& Identifier)
{
	if (const int32 WorkingIndex = GetIdentifierIndex(Identifier); 
		WorkingIndex != INDEX_NONE)
	{
		Identifiers.RemoveAt(WorkingIndex);
		Templates.RemoveAt(WorkingIndex);
		Payloads.RemoveAt(WorkingIndex);
		
		SaveConfig();
	}
	else
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidgetUserSettings::UnregisterWidget] Unable to find widget with identifier %s"), *Identifier.ToString());
	}
}

void UNEditorUtilityWidgetUserSettings::UpdatePayload(const FName& Identifier, const FNEditorUtilityWidgetUserSettingsPayload& Payload)
{
	if (const int32 WorkingIndex = GetIdentifierIndex(Identifier); 
		WorkingIndex != INDEX_NONE)
	{
		Payloads[WorkingIndex] = Payload;
		SaveConfig();
	}
	else
	{
		NE_LOG(Error, TEXT("[UNEditorUtilityWidgetUserSettings::UpdatePayload] Unable to find widget with identifier %s"), *Identifier.ToString());
	}
}
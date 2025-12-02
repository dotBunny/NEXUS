#include "NEditorUtilityWidgetUserSettings.h"

void UNEditorUtilityWidgetUserSettings::OnPostEngineInit()
{
	if (UNEditorUtilityWidgetUserSettings* Settings = GetMutable())
	{
		TArray<FNEditorUtilityWidgetSettings> Copy = Settings->Widgets;
		Settings->Widgets.Empty();
		
		for (FNEditorUtilityWidgetSettings WidgetSettings : Copy)
		{
			// TODO: Recreate widgets (this will again add them back)
		}
	}
}

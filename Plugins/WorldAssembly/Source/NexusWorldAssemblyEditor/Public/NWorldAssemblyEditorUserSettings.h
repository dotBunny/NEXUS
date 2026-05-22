// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NWorldAssemblyEditorUserSettings.generated.h"

/**
 * Per-user editor preferences for NexusWorldAssembly. Stored in NexusUserSettings.ini so each developer
 * keeps their own values.
 */
UCLASS(config = NexusUserSettings, meta = (DisplayName = "World Assembly (User)"))
class NEXUSWORLDASSEMBLYEDITOR_API UNWorldAssemblyEditorUserSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNWorldAssemblyEditorUserSettings, "World Assembly (User)", "Specific settings for NEXUS: World Assembly included with the framework.");
	
#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, config,  Category="Cell", DisplayName="Display Viewport Messages", meta=(Tooltip="Show alerts and messages in the viewport while editing Cells."))
	bool bCellDisplayViewportMessages = true;

#endif // WITH_EDITORONLY_DATA
};
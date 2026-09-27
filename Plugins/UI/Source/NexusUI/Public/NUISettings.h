// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NSettingsUtils.h"
#include "Macros/NSettingsMacros.h"
#include "NUISettings.generated.h"

/**
 * Project-wide configuration for the NexusUI plugin.
 *
 * Exposed under Project Settings > NEXUS > User Interface; stored in DefaultNexusGame.ini.
 * @see <a href="https://nexus-framework.com/docs/ui/project-settings/">UNUISettings</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "User Interface Settings", Config=NexusGame, defaultconfig)
class NEXUSUI_API UNUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

	N_SETTINGS_BASE(UNUISettings, "User Interface", "Settings related to the User Interface.")

public:
	/** Create each text render's font materials on the game thread before a loading or streaming level registers it; see UNTextRenderWarmupSubsystem. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Text Render", DisplayName = "Warm Text Render Materials",
		meta=(ToolTip="Create each text render's font materials on the game thread before a loading or streaming level registers it, rather than on the worker that registers the level. Read when a world is created."))
	bool bWarmTextRenderMaterials = true;
};

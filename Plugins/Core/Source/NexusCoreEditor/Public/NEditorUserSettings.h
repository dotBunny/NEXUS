// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NEditorUserSettings.generated.h"

/**
 * Per-user NEXUS editor preferences persisted outside project config.
 *
 * Stores machine-local state (such as the last time the update-check ran) so it does not leak
 * into source control via the shared NexusEditor ini.
 */
UCLASS(config = NexusUserSettings)
class NEXUSCOREEDITOR_API UNEditorUserSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_HIDDEN_EDITOR_SETTINGS(UNEditorUserSettings);


#if WITH_EDITORONLY_DATA

	/** Timestamp of the most recent successful update check; compared against UNEditorSettings::UpdatesFrequency. */
	UPROPERTY(Config)
	FDateTime UpdatesLastChecked = FDateTime(2025, 7, 12);

#endif // WITH_EDITORONLY_DATA
};
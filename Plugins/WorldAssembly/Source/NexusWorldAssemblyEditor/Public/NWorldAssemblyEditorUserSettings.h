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
	N_EDITOR_SETTINGS_BASE(UNWorldAssemblyEditorUserSettings, "World Assembly (User)", "Specific settings for NEXUS: World Assembly included with the framework.");
	
#if WITH_EDITORONLY_DATA

	/** Show alerts and HUD messages in the viewport while editing cells. */
	UPROPERTY(EditAnywhere, config,  Category="Cell", DisplayName="Display Viewport Messages", meta=(Tooltip="Show alerts and messages in the viewport while editing Cells."))
	bool bCellDisplayViewportMessages = true;

	UPROPERTY(EditAnywhere, config,  Category="Operations", DisplayName="Show Quick Assembly Section", meta=(Tooltip="Shows a drop down to select the organ to run a World Assembly on."))
	bool bShowQuickAssemblySection = true;
	
	/** Draw debug markers for unfilled (unconnected) junctions in the world preview. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,  Category = "Junctions", DisplayName="Draw Unfilled Junctions")
	bool bDebugWorldDrawUnfilledJunctions = true;

	/** Color used to draw unfilled junction markers when bDebugWorldDrawUnfilledJunctions is enabled. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Junctions", DisplayName="Unfilled Junctions Color")
	FLinearColor EmptyJunctionColor = FLinearColor::Gray;
	
#endif // WITH_EDITORONLY_DATA
};
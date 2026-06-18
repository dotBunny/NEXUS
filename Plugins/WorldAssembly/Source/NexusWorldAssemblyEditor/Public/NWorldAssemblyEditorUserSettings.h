// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "NWorldAssemblyEdMode.h"
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

	/** Show the Quick Assembly section (Organ dropdown + start/cancel button) on the World Assembly toolbar. */
	UPROPERTY(EditAnywhere, config,  Category="Operations", DisplayName="Show Quick Assembly Section", meta=(Tooltip="Shows a drop down to select the organ to run a World Assembly on."))
	bool bShowQuickAssemblySection = true;
	
	/** Show alerts and HUD messages in the viewport while editing cells. */
	UPROPERTY(EditAnywhere, config,  Category="Cell", DisplayName="Display Viewport Messages", meta=(Tooltip="Show alerts and messages in the viewport while editing Cells."))
	bool bCellDisplayViewportMessages = true;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,  Category = "Cell|Colors", DisplayName="Bounds")
	FLinearColor CellBoundsColor = NEXUS::WorldAssembly::DefaultColors::CellBounds;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,  Category = "Cell|Colors", DisplayName="Hull")
	FLinearColor CellHullColor = NEXUS::WorldAssembly::DefaultColors::CellHull;
	
	/** Draw debug markers for unfilled (unconnected) junctions in the world preview. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,  Category = "Junctions", DisplayName="Draw Unfilled Junctions")
	bool bDebugWorldDrawUnfilledJunctions = true;
	
	/** Color used to draw unfilled junction markers when bDebugWorldDrawUnfilledJunctions is enabled. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Junctions|Colors", DisplayName="Unfilled", meta=(Tooltip="Color of junction when it is unfilled during a world assembly operation."))
	FLinearColor JunctionsUnfilledColor = NEXUS::WorldAssembly::DefaultColors::JunctionUnfilled;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Junctions|Colors", DisplayName="Valid")
	FLinearColor JunctionsValidColor = NEXUS::WorldAssembly::DefaultColors::JunctionValid;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Junctions|Colors", DisplayName="Invalid", meta=(Tooltip="Color of junction when it is embedded too far into geometry making it impossible to match."))
	FLinearColor JunctionsInvalidColor = NEXUS::WorldAssembly::DefaultColors::JunctionInvalid;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Bones|Colors", DisplayName="Valid")
	FLinearColor BonesValidColor = NEXUS::WorldAssembly::DefaultColors::BoneValid;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Bones|Colors", DisplayName="Invalid")
	FLinearColor BonesInvalidColor = NEXUS::WorldAssembly::DefaultColors::BoneInvalid;
	
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);

		const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

		// KEEP SYNCED WITH FNWorldAssemblyEditorUserSettingsCustomization
		// Is this a cached property needing to be cached
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UNWorldAssemblyEditorUserSettings, BonesValidColor) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UNWorldAssemblyEditorUserSettings, BonesInvalidColor) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UNWorldAssemblyEditorUserSettings, CellBoundsColor) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UNWorldAssemblyEditorUserSettings, CellHullColor) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UNWorldAssemblyEditorUserSettings, JunctionsUnfilledColor) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UNWorldAssemblyEditorUserSettings, JunctionsValidColor) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UNWorldAssemblyEditorUserSettings, JunctionsInvalidColor))
		{
			FNWorldAssemblyEdMode::CacheUserSettings();
		}
	}
	
	virtual void PostEditUndo() override
	{
		FNWorldAssemblyEdMode::CacheUserSettings();
	}

#endif // WITH_EDITORONLY_DATA
};
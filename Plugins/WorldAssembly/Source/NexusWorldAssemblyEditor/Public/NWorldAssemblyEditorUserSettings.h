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

	/** Color used to draw bones that are in a valid (matchable) state in the world preview. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Color Palette|Bones", DisplayName="Valid")
	FLinearColor ColorPaletteBonesValid = NEXUS::WorldAssembly::DefaultColors::BoneValid;

	/** Color used to draw bones that are in an invalid (unmatchable) state in the world preview. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Color Palette|Bones", DisplayName="Invalid")
	FLinearColor ColorPaletteBonesInvalid = NEXUS::WorldAssembly::DefaultColors::BoneInvalid;

	/** Color used to draw the bounding box of a cell in the world preview. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,  Category = "Color Palette|Cell", DisplayName="Bounds")
	FLinearColor ColorPaletteCellBounds = NEXUS::WorldAssembly::DefaultColors::CellBounds;

	/** Color used to draw the convex hull of a cell in the world preview. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,  Category = "Color Palette|Cell", DisplayName="Hull")
	FLinearColor ColorPaletteCellHull = NEXUS::WorldAssembly::DefaultColors::CellHull;

	/** Color used to draw unfilled junction markers when bDebugWorldDrawUnfilledJunctions is enabled. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Color Palette|Junctions", DisplayName="Unfilled", meta=(Tooltip="Color of junction when it is unfilled during a world assembly operation."))
	FLinearColor ColorPaletteJunctionsUnfilled = NEXUS::WorldAssembly::DefaultColors::JunctionUnfilled;

	/** Color used to draw junctions that are in a valid (matchable) state in the world preview. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Color Palette|Junctions", DisplayName="Valid")
	FLinearColor ColorPaletteJunctionsValid = NEXUS::WorldAssembly::DefaultColors::JunctionValid;

	/** Color used to draw a junction that is embedded too far into geometry, making it impossible to match. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Color Palette|Junctions", DisplayName="Invalid", meta=(Tooltip="Color of junction when it is embedded too far into geometry making it impossible to match."))
	FLinearColor ColorPaletteJunctionsInvalid = NEXUS::WorldAssembly::DefaultColors::JunctionInvalid;

	/** Draw debug markers for unfilled (unconnected) junctions in the world preview. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,  Category = "Debug", DisplayName="Draw Unfilled Junctions")
	bool bDebugWorldDrawUnfilledJunctions = true;

	/** Show alerts and HUD messages in the viewport while editing cells. */
	UPROPERTY(EditAnywhere, config,  Category="Notifications", DisplayName="Display Viewport Messages", meta=(Tooltip="Shows relevant alerts and messages in the viewport upper-left corner."))
	bool bNotificationsDisplayViewportMessages = true;

	/** Show a toast notification when an editor-triggered Assembly Operation completes, and summarize Quick Assembly runs. */
	UPROPERTY(EditAnywhere, config,  Category="Notifications", DisplayName="Toast Editor Assembly Operations", meta=(Tooltip="Show a toast notification when an editor-triggered Assembly Operation completes and/or summarize Quick Assembly operations."))
	bool bNotificationsToastEditorAssemblyOperations = true;

	/** Create and load the level instances from the ANCellProxy(s) produced by a Quick Assembly. */
	UPROPERTY(EditAnywhere, config,  Category="Quick Assembly", DisplayName="Load Level Instances", meta=(Tooltip="Create and load the level instances from the ANCellProxy(s)."))
	bool bQuickAssemblyLoadLevelInstances = true;

	/** Continuously re-trigger Assembly Operations for the target organ on a timer until cancelled. */
	UPROPERTY(EditAnywhere, config,  Category="Quick Assembly", DisplayName="Auto Assembly", meta=(Tooltip="Continuously trigger Assembly Operations for the target organ on a timer, until cancelled."))
	bool bQuickAssemblyAutoAssembly = false;

	/** Seconds to wait after a run completes before the next Auto Assembly is triggered (only used when bQuickAssemblyAutoAssembly is enabled). */
	UPROPERTY(EditAnywhere, config,  Category="Quick Assembly", DisplayName="Auto Assembly Timer",
		meta=(Tooltip="Continuously trigger Assembly Operations for the target organ on a timer, post completion, until cancelled.",
			ClampMin="2", ClampMax="180", UIMin="2", UIMax="180", SliderExponent = 0.5))
	float QuickAssemblyAutoAssemblyTimer = 2.f;

	//~UObject
	/** Re-cache the color palette on the edit mode whenever one of the cached color properties is changed in the editor. */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);

		const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

		// KEEP SYNCED WITH FNWorldAssemblyEditorUserSettingsCustomization
		// Is this a cached property needing to be cached
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UNWorldAssemblyEditorUserSettings, ColorPaletteBonesValid) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UNWorldAssemblyEditorUserSettings, ColorPaletteBonesInvalid) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UNWorldAssemblyEditorUserSettings, ColorPaletteCellBounds) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UNWorldAssemblyEditorUserSettings, ColorPaletteCellHull) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UNWorldAssemblyEditorUserSettings, ColorPaletteJunctionsUnfilled) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UNWorldAssemblyEditorUserSettings, ColorPaletteJunctionsValid) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UNWorldAssemblyEditorUserSettings, ColorPaletteJunctionsInvalid))
		{
			FNWorldAssemblyEdMode::CacheUserSettings();
		}
	}

	/** Re-cache the color palette on the edit mode after an undo/redo, since the changed properties bypass PostEditChangeProperty. */
	virtual void PostEditUndo() override
	{
		FNWorldAssemblyEdMode::CacheUserSettings();
	}
	//End UObject

#endif // WITH_EDITORONLY_DATA
};
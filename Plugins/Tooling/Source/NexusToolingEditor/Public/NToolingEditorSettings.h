// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "MultiplayerTest/NMultiplayerTestToolbarSection.h"
#include "NToolingEditorSettings.generated.h"

/**
 * Per-validator severity used by NexusTooling validators to decide how to respond to a match.
 * Drives both the UE data-validation verdict (Valid/Invalid/NotValidated) and the message bus.
 */
UENUM(BlueprintType)
enum class ENValidatorSeverity : uint8
{
	Disable	= 0,
	Warning = 1,
	WarningButValid = 2 UMETA(DisplayName = "Warning (Validate)", ToolTip="Throws warning but does not mark the asset as invalid."),
	Error = 3,
	Message = 4
};

/**
 * Project-wide editor settings for NexusTooling.
 *
 * Shipped in NexusEditor.ini so they travel with the project — per-user overrides live on
 * UNToolingEditorUserSettings instead. Groups icon overrides, level bookmarks, and per-validator
 * severities.
 */
UCLASS(config = NexusEditor, defaultconfig)
class UNToolingEditorSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNToolingEditorSettings, "Tooling", "Specific settings for NEXUS: Tooling included with the framework.");

#if WITH_EDITOR
	/** @return true if AssetPath matches ValidatorIgnoredAssets or starts with an entry in ValidatorIgnoredPrefixes. */
	bool IsAssetIgnored(const FSoftObjectPath& AssetPath) const;
	
	//~UObject
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
		const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UNToolingEditorSettings, bMultiplayerTestEnabled))
		{
			const bool bHasMultiplayerTestEnabled = FNMultiplayerTestToolbarSection::HasSection();
			if (bMultiplayerTestEnabled && !bHasMultiplayerTestEnabled)
			{
				FNMultiplayerTestToolbarSection::AddSection();
			}
			else if (!bMultiplayerTestEnabled && bHasMultiplayerTestEnabled)
			{
				FNMultiplayerTestToolbarSection::RemoveSection();
			}
		}
	}
	//End UObject
#endif // WITH_EDITOR

#if WITH_EDITORONLY_DATA
	
	UPROPERTY(EditAnywhere, config, Category = "Editor Icon", DisplayName = "AppIcon Path", meta=(ToolTip="Replaces the Starship AppIcon style, can be in SVG or image format. A restart of the editor is required for it to take effect."))
	FString ProjectAppIconPath;

	UPROPERTY(EditAnywhere, config, Category = "Editor Icon", DisplayName = "Window Icon Path", meta=(ToolTip="Replaces the Unreal Editor window icon at a platform level. This should simply be the path to the file WITHOUT any extension. The extension will be determined by the platform, thus all resources should have the same base name, and be located in the same folder. A restart of the editor is required for it to take effect."))
	FString ProjectWindowIconPath;
	
	UPROPERTY(EditAnywhere, config, Category = "Project", DisplayName = "Levels", meta = (AllowedClasses = "/Script/Engine.World", ToolTip="The set of project levels referenced by NEXUS tooling."))
	TArray<FSoftObjectPath> ProjectLevels;

	UPROPERTY(EditAnywhere, config, Category = "Validators|Severity", DisplayName = "Blueprint: Empty Tick", meta=(ToolTip="Severity reported when a Blueprint contains an empty Tick event."))
	ENValidatorSeverity ValidatorBlueprintEmptyTick = ENValidatorSeverity::Error;

	UPROPERTY(EditAnywhere, config, Category = "Validators|Severity", DisplayName = "Blueprint: Multi-Pin Pure Node", meta=(ToolTip="Severity reported when a Blueprint pure node has multiple connected output pins (which causes it to be re-evaluated per pin)."))
	ENValidatorSeverity ValidatorBlueprintMultiPinPureNode = ENValidatorSeverity::Warning;

	UPROPERTY(EditAnywhere, config, Category = "Validators|Severity", DisplayName = "Engine: Content Change", meta=(ToolTip="Severity reported when engine content has been modified."))
	ENValidatorSeverity ValidatorEngineContentChange = ENValidatorSeverity::Warning;

	UPROPERTY(EditAnywhere, config, Category = "Validators|Severity", DisplayName = "Level: Blueprint Logic",
		meta=(Tooltip="Determine if there are any non-ghost nodes in the level blueprint."))
	ENValidatorSeverity ValidatorLevelBlueprint = ENValidatorSeverity::Disable;
	
	UPROPERTY(EditAnywhere, config, Category = "Validators|Ignored", DisplayName = "Ignored Assets", meta=(ToolTip="Specific assets excluded from all validators."))
	TArray<FSoftObjectPath> ValidatorIgnoredAssets;
	
	UPROPERTY(EditAnywhere, config, Category = "Validators|Ignored", DisplayName = "Ignored Prefixes",
		meta = (ToolTip = "This can be folder paths which will ignore by looking if a path starts with this."))
	TArray<FString> ValidatorIgnoredPrefixes;
	
	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test", DisplayName = "Enabled", meta=(ToolTip="Should the Multiplayer Test be enabled?"))
	bool bMultiplayerTestEnabled = true;
	
	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test", meta = (DisplayName = "Use Online Subsystem", Tooltip = "Should authentication use the Online Subsystem?"))
	bool bMultiplayerTestUseOnlineSubsystem = false;
	
	/**
	 * Forwards the project-level multiplayer-test toggles onto the supplied play-session request.
	 * @param Params The play-session request whose Online Subsystem behavior should be set.
	 */
	void ApplySettings(FRequestPlaySessionParams& Params) const
	{
		Params.bAllowOnlineSubsystem = bMultiplayerTestUseOnlineSubsystem;
	}
#endif // WITH_EDITORONLY_DATA

};
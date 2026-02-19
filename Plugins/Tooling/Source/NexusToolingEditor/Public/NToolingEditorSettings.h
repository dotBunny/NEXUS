// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NEditorUtils.h"
#include "NToolingEditorSettings.generated.h"

UENUM(BlueprintType)
enum class ENValidatorSeverity : uint8
{
	Disable	= 0,
	Warning = 1,
	WarningButValid = 2 UMETA(DisplayName = "Warning (Validate)", ToolTip="Throws warning but does not mark the asset as invalid."),
	Error = 3,
	Message = 4
};

UCLASS(config = NexusEditor, defaultconfig)
class UNToolingEditorSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNToolingEditorSettings);
	
	virtual FName GetContainerName() const override { return FNEditorDefaults::GetEditorSettingsContainerName(); }
	virtual FName GetCategoryName() const override {  return FNEditorDefaults::GetEditorSettingsCategoryName();  }
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Tooling"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("A collection of tools for fixing content in the Unreal Editor."));
		return SectionDescription;
	}
	
#if WITH_EDITOR	
	bool IsAssetIgnored(const FSoftObjectPath& AssetPath) const;
#endif // WITH_EDITOR

#if WITH_EDITORONLY_DATA
	
	UPROPERTY(EditAnywhere, config, Category = "Editor Icon", DisplayName = "AppIcon Path", meta=(ToolTip="Replaces the Starship AppIcon style, can be in SVG or image format. A restart of the editor is required for it to take effect."))
	FString ProjectAppIconPath;

	UPROPERTY(EditAnywhere, config, Category = "Editor Icon", DisplayName = "Window Icon Path", meta=(ToolTip="Replaces the Unreal Editor window icon at a platform level. This should simply be the path to the file WITHOUT any extension. The extension will be determined by the platform, thus all resources should have the same base name, and be located in the same folder. A restart of the editor is required for it to take effect."))
	FString ProjectWindowIconPath;
	
	UPROPERTY(EditAnywhere, config, Category = "Project", DisplayName = "Levels", meta = (AllowedClasses = "/Script/Engine.World"))
	TArray<FSoftObjectPath> ProjectLevels;

	UPROPERTY(EditAnywhere, config, Category = "Validators: Severity", DisplayName = "Blueprint: Empty Tick")
	ENValidatorSeverity ValidatorBlueprintEmptyTick = ENValidatorSeverity::Error;
	
	UPROPERTY(EditAnywhere, config, Category = "Validators: Severity", DisplayName = "Blueprint: Multi-Pin Pure Node")
	ENValidatorSeverity ValidatorBlueprintMultiPinPureNode = ENValidatorSeverity::Warning;

	UPROPERTY(EditAnywhere, config, Category = "Validators: Severity", DisplayName = "Engine: Content Change")
	ENValidatorSeverity ValidatorEngineContentChange = ENValidatorSeverity::Warning;

	UPROPERTY(EditAnywhere, config, Category = "Validators: Ignored", DisplayName = "Ignored Assets")
	TArray<FSoftObjectPath> ValidatorIgnoredAssets;
	
	UPROPERTY(EditAnywhere, config, Category = "Validators: Ignored", DisplayName = "Ignored Prefixes",
		meta = (ToolTip = "This can be folder paths which will ignore by looking if a path starts with this."))
	TArray<FString> ValidatorIgnoredPrefixes;
	
#endif // WITH_EDITORONLY_DATA

};
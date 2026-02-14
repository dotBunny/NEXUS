// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NEditorUtils.h"
#include "NToolsEditorSettings.generated.h"

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
class UNToolsEditorSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNToolsEditorSettings);
	
	static void Register() { FNEditorUtils::RegisterSettings(GetMutable());}
	static void Unregister() { FNEditorUtils::UnregisterSettings(Get());}
	
	virtual FName GetContainerName() const override { return FNEditorDefaults::GetEditorSettingsContainerName(); }
	virtual FName GetCategoryName() const override {  return FNEditorDefaults::GetEditorSettingsCategoryName();  }
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Tools"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("A collection of tools for fixing content in the Unreal Editor."));
		return SectionDescription;
	}
	
	bool IsAssetIgnored(const FSoftObjectPath& AssetPath) const;

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, config, Category = "Validators", DisplayName = "Blueprint: Empty Tick")
	ENValidatorSeverity BlueprintEmptyTick = ENValidatorSeverity::Error;
	
	UPROPERTY(EditAnywhere, config, Category = "Validators", DisplayName = "Blueprint: Multi-Pin Pure Node")
	ENValidatorSeverity BlueprintMultiPinPureNode = ENValidatorSeverity::Warning;

	UPROPERTY(EditAnywhere, config, Category = "Validators", DisplayName = "Engine: Content Change")
	ENValidatorSeverity EngineContentChange = ENValidatorSeverity::Warning;

	UPROPERTY(EditAnywhere, config, Category = "Validator Ignores", DisplayName = "Assets")
	TArray<FSoftObjectPath> IgnoredAssets;
	
	UPROPERTY(EditAnywhere, config, Category = "Validator Ignores", DisplayName = "Prefixes",
		meta = (ToolTip = "This can be folder paths which will ignore by looking if a path starts with this."))
	TArray<FString> IgnoredPrefixes;
	
#endif // WITH_EDITORONLY_DATA

};
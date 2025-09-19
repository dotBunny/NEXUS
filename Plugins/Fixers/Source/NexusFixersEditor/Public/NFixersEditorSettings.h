// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NEditorUtils.h"
#include "NFixersEditorSettings.generated.h"


UENUM(BlueprintType)
enum ENValidatorSeverity : uint8
{
	NVS_Disable	= 0 						UMETA(DisplayName = "Disabled"),
	NVS_Warning = 1							UMETA(DisplayName = "Warning"),
	NVS_WarningButValid = 2					UMETA(DisplayName = "Warning (Validate)", ToolTip="Throws warning but does not mark the asset as invalid."),
	NVS_Error = 3							UMETA(DisplayName = "Error"),
	NVS_Message = 4							UMETA(DisplayName = "Message")
};

UCLASS(config = NexusEditor, defaultconfig)
class UNFixersEditorSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNFixersEditorSettings);
	
	static void Register() { FNEditorUtils::RegisterSettings(GetMutable());}
	static void Unregister() { FNEditorUtils::UnregisterSettings(Get());}
	
	virtual FName GetContainerName() const override { return FNEditorDefaults::GetEditorSettingsContainerName(); }
	virtual FName GetCategoryName() const override {  return FNEditorDefaults::GetEditorSettingsCategoryName();  }
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Fixers"));
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
	TEnumAsByte<ENValidatorSeverity> BlueprintEmptyTick = NVS_Error;
	
	UPROPERTY(EditAnywhere, config, Category = "Validators", DisplayName = "Blueprint: Multi-Pin Pure Node")
	TEnumAsByte<ENValidatorSeverity> BlueprintMultiPinPureNode = NVS_Warning;

	UPROPERTY(EditAnywhere, config, Category = "Validators", DisplayName = "Engine: Content Change")
	TEnumAsByte<ENValidatorSeverity> EngineContentChange = NVS_Warning;

	UPROPERTY(EditAnywhere, config, Category = "Validator Ignores", DisplayName = "Assets")
	TArray<FSoftObjectPath> IgnoredAssets;
	UPROPERTY(EditAnywhere, config, Category = "Validator Ignores", DisplayName = "Prefixes",
		meta = (ToolTip = "This can be folder paths which will ignore by looking if a path starts with this."))
	TArray<FString> IgnoredPrefixes;
#endif
};
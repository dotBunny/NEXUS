// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NMultiplayerEditorCommands.h"
#include "NMultiplayerEditorSettings.generated.h"

/**
 * Project-wide editor settings for NexusMultiplayerEditor.
 *
 * Shipped in NexusEditor.ini so they travel with the project — per-user overrides live on
 * UNMultiplayerEditorUserSettings instead.
 */
UCLASS(config = NexusEditor, defaultconfig)
class UNMultiplayerEditorSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNMultiplayerEditorSettings);

	//~UDeveloperSettings
	virtual FName GetContainerName() const override { return FNEditorDefaults::GetEditorSettingsContainerName(); }
	virtual FName GetCategoryName() const override {  return FNEditorDefaults::GetEditorSettingsCategoryName();  }
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Multiplayer"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("Configuration for the multiplayer test system."));
		return SectionDescription;
	}
	//End UDeveloperSettings
	
#if WITH_EDITOR
	//~UObject
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
		const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UNMultiplayerEditorSettings, bMultiplayerTestEnabled))
		{
			const bool bHasMultiplayerTestEnabled = FNMultiplayerEditorCommands::HasMultiplayerTestSection();
			if (bMultiplayerTestEnabled && !bHasMultiplayerTestEnabled)
			{
				FNMultiplayerEditorCommands::AddMultiplayerTestSection();
			}
			else if (!bMultiplayerTestEnabled && bHasMultiplayerTestEnabled)
			{
				FNMultiplayerEditorCommands::RemoveMultiplayerTestSection();
			}
		}
	}
	//End UObject
#endif // WITH_EDITOR	

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test", DisplayName = "Enabled", meta=(ToolTip="Should the Multiplayer Test be enabled?"))
	bool bMultiplayerTestEnabled = true;
	
	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test", meta = (DisplayName = "Use Online Subsystem", Tooltip = "Should authentication use the Online Subsystem?"))
	bool bMultiplayerTestUseOnlineSubsystem = false;
	
	void ApplySettings(FRequestPlaySessionParams& Params) const
	{
		Params.bAllowOnlineSubsystem = bMultiplayerTestUseOnlineSubsystem;
	}
#endif // WITH_EDITORONLY_DATA	
};
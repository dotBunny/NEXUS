// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NEditorUtils.h"
#include "NMultiplayerEditorUserSettings.generated.h"

UCLASS(config = NexusUserSettings, meta = (DisplayName = "Multiplayer (User)"))
class UNMultiplayerEditorUserSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNMultiplayerEditorUserSettings);

	static void Register() { FNEditorUtils::RegisterSettings(GetMutable()); }
	static void Unregister() { FNEditorUtils::UnregisterSettings(Get());}
	
	virtual FName GetContainerName() const override { return FNEditorDefaults::GetEditorSettingsContainerName(); }
	virtual FName GetCategoryName() const override { return FNEditorDefaults::GetEditorSettingsCategoryName();  }
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Multiplayer (User)"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("Configuration for the multiplayer test system."));
		return SectionDescription;
	}

#if WITH_EDITORONLY_DATA
	
	UPROPERTY(EditAnywhere, config, Category = "Setup", meta = (DisplayName = "Use Online Subsystem", Tooltip = "Should authentication use the Online Subsystem?"))
	bool bUseOnlineSubsystem = false;
	
	UPROPERTY(EditAnywhere, config, Category = "Clients", meta = (DisplayName = "Count", Tooltip = "The number of clients to spawn."))
	int ClientCount = 2;

	UPROPERTY(EditAnywhere, config, Category = "Clients", meta = (DisplayName = "Window Size", Tooltip = "The size of the created clients windows."))
	FIntPoint ClientWindowSize = FIntPoint(800, 600);

	UPROPERTY(EditAnywhere, config, Category = "Clients", meta = (DisplayName = "Generate Network Profile", Tooltip = "Profiles will be stored in <PROJECT_DIRECTORY>/Saved/Profiling/<PROJECT_NAME>-<TIMESTAMP>.nprof"))
	bool bClientGenerateNetworkProfile = false;
	
	UPROPERTY(EditAnywhere, config, Category = "Clients", meta = (DisplayName = "Parameters", Tooltip = "Additional parameters to pass to the client being launched."))
	FString ClientParameters = "";
	
	UPROPERTY(EditAnywhere, config, Category = "Server", meta = (EditCondition="!bClientGenerateNetworkProfile", EditConditionHides, DisplayName = "Generate Network Profile", Tooltip = "This setting has no effect if option is turned on for clients. Profiles will be stored in <PROJECT_DIRECTORY>/Saved/Profiling/<PROJECT_NAME>-<TIMESTAMP>.nprof"))
	bool bServerGenerateNetworkProfile = false;
	
	UPROPERTY(EditAnywhere, config, Category = "Server", meta = (DisplayName = "Dedicated Server", Tooltip = "Spawn a dedicated server for use by the clients."))
	bool bUseDedicatedServer = true;

	UPROPERTY(EditAnywhere, config, Category = "Server", meta = (DisplayName = "Spawn Seperate Server", Tooltip = "This is a rarely used option that will launch a separate server (possibly hidden in-process depending on RunUnderOneProcess) even if the net mode does not require a server (such as Standalone). If the net mode requires a server (such as Client) a server will be launched for you (regardless of this setting)."))
	bool bSpawnSeparateServer = false;

	UPROPERTY(EditAnywhere, config, Category = "Server", meta = (DisplayName = "Parameters", Tooltip = "Additional parameters to pass to the server being launched, on top of client parameters."))
	FString ServerParameters = "";

#endif // WITH_EDITORONLY_DATA
};
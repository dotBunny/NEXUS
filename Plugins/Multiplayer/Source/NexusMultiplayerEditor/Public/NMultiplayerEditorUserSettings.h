// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NMultiplayerEditorUserSettings.generated.h"

UCLASS(config = NexusUserSettings, meta = (DisplayName = "Multiplayer (User)"))
class UNMultiplayerEditorUserSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNMultiplayerEditorUserSettings);
	
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
	
	UPROPERTY(EditAnywhere, config, Category = "Setup", meta = (DisplayName = "Clear Logs Folder", Tooltip = "Clear the log files (not in use) prior to running a test?"))
	bool bClearLogsFolder = false;
	
	UPROPERTY(EditAnywhere, config, Category = "Clients", meta = (DisplayName = "Count", Tooltip = "The number of clients to spawn."))
	int ClientCount = 2;

	UPROPERTY(EditAnywhere, config, Category = "Clients", meta = (DisplayName = "Window Size", Tooltip = "The size of the created clients windows."))
	FIntPoint ClientWindowSize = FIntPoint(800, 600);

	UPROPERTY(EditAnywhere, config, Category = "Clients", meta = (DisplayName = "Disable Sound", Tooltip = "Mutes sound of clients."))
	bool bClientDisableSound = false;
	
	UPROPERTY(EditAnywhere, config, Category = "Clients", meta = (DisplayName = "Generate Network Profile", Tooltip = "Profiles will be stored in <PROJECT_DIRECTORY>/Saved/Profiling/<PROJECT_NAME>-<TIMESTAMP>.nprof"))
	bool bClientGenerateNetworkProfile = false;
	
	UPROPERTY(EditAnywhere, config, Category = "Clients", meta = (DisplayName = "Parameters", Tooltip = "Additional parameters to pass to the client being launched."))
	FString ClientParameters = "";
	
	UPROPERTY(EditAnywhere, config, Category = "Clients: Network Simulation", meta = (DisplayName = "Lag (Minimum)", Tooltip = "The minimum amount of network lag (ms) be simulated ontop of the existing round trip time.", ClampMin="0", ClampMax="1000", UIMin="0", UIMax="1000", SliderExponent = 1))
	int ClientSimulateLagMinimum = 20;
	
	UPROPERTY(EditAnywhere, config, Category = "Clients: Network Simulation", meta = (DisplayName = "Lag (Maximum)", Tooltip = "The maximum amount of network lag (ms) be simulated ontop of the existing round trip time.", ClampMin="0", ClampMax="1000", UIMin="0", UIMax="1000", SliderExponent = 1))
	int ClientSimulateLagMaximum = 60;
	
	UPROPERTY(EditAnywhere, config, Category = "Clients: Network Simulation", meta = (DisplayName = "Packet Loss", Tooltip = "An amount of packet loss (%) to be simulated.", ClampMin="0", ClampMax="100", UIMin="0", UIMax="100", SliderExponent = 1)) 
	int ClientSimulatePacketLoss = 0;
	
	UPROPERTY(EditAnywhere, config, Category = "Clients: Network Simulation", meta = (DisplayName = "Packet Jitter", Tooltip = "An amount of packet jitter (ms) to be simulated.", ClampMin="0", ClampMax="1000", UIMin="0", UIMax="1000", SliderExponent = 1)) 
	int ClientSimulatePacketJitter = 0;
	
	UPROPERTY(EditAnywhere, config, Category = "Clients: Network Simulation", meta = (DisplayName = "Packet Duplication", Tooltip = "An amount of packet duplication (%) to be simulated.", ClampMin="0", ClampMax="100", UIMin="0", UIMax="100", SliderExponent = 1)) 
	int ClientSimulatePacketDuplication = 0;
	
	UPROPERTY(EditAnywhere, config, Category = "Clients: Network Simulation", meta = (DisplayName = "Receive Out Of Order", Tooltip = "Forces network packets to be recieved out of order.")) 
	bool bClientSimulateReceiveOutOfOrderPackets = false;

	UPROPERTY(EditAnywhere, config, Category = "Server", meta = (EditCondition="!bClientGenerateNetworkProfile", EditConditionHides, DisplayName = "Generate Network Profile", Tooltip = "This setting has no effect if option is turned on for clients. Profiles will be stored in <PROJECT_DIRECTORY>/Saved/Profiling/<PROJECT_NAME>-<TIMESTAMP>.nprof"))
	bool bServerGenerateNetworkProfile = false;
	
	UPROPERTY(EditAnywhere, config, Category = "Server", meta = (DisplayName = "Dedicated Server", Tooltip = "Spawn a dedicated server for use by the clients."))
	bool bUseDedicatedServer = true;

	UPROPERTY(EditAnywhere, config, Category = "Server", meta = (DisplayName = "Spawn Seperate Server", Tooltip = "This is a rarely used option that will launch a separate server (possibly hidden in-process depending on RunUnderOneProcess) even if the net mode does not require a server (such as Standalone). If the net mode requires a server (such as Client) a server will be launched for you (regardless of this setting)."))
	bool bSpawnSeparateServer = false;

	UPROPERTY(EditAnywhere, config, Category = "Server", meta = (DisplayName = "Parameters", Tooltip = "Additional parameters to pass to the server being launched, on top of client parameters."))
	FString ServerParameters = "";

private:
	FString GetClientArguments() const
	{
		FString ClientArguments = TEXT("-NMultiplayerTest");
		
		if (bClientGenerateNetworkProfile)
		{
			// ReSharper disable once StringLiteralTypo
			ClientArguments.Append(" networkprofiler=true");
		}
		
		if (bClientDisableSound)
		{
			// ReSharper disable once StringLiteralTypo
			ClientArguments.Append(" -nosound");
		}
		if (ClientSimulateLagMinimum > 0)
		{
			ClientArguments.Append(FString::Printf(TEXT(" -PktLagMin=%i"), FMath::FloorToInt(ClientSimulateLagMinimum * 0.5f)));
		}
		else if (ClientSimulateLagMaximum > 0)
		{
			ClientArguments.Append(" -PktLagMin=0");
		}
		
		if (ClientSimulateLagMaximum > 0)
		{
			ClientArguments.Append(FString::Printf(TEXT(" -PktLagMax=%i"), FMath::FloorToInt(ClientSimulateLagMaximum * 0.5f)));
		}
		
		if (ClientSimulatePacketLoss > 0)
		{
			ClientArguments.Append(FString::Printf(TEXT(" -PktLoss=%i"), ClientSimulatePacketLoss));
		}
		
		if (ClientSimulatePacketDuplication > 0)
		{
			ClientArguments.Append(FString::Printf(TEXT(" -PktDup=%i"), ClientSimulatePacketDuplication));
		}
		
		if (bClientSimulateReceiveOutOfOrderPackets)
		{
			ClientArguments.Append(" -PktOrder=1");
		}
		
		ClientArguments.Append(FString::Printf(TEXT(" %s"), *ClientParameters.TrimStartAndEnd()));
		
		return MoveTemp(ClientArguments);
	}
	
	FString GetServerArguments() const
	{
		FString ServerAdditionalArguments = TEXT("");
		
		if (bServerGenerateNetworkProfile)
		{
			// ReSharper disable once StringLiteralTypo
			ServerAdditionalArguments.Append(" networkprofiler=true");
		}
		
		ServerAdditionalArguments.Append(FString::Printf(TEXT(" %s"), *ServerAdditionalArguments.TrimStartAndEnd()));
		
		return MoveTemp(ServerAdditionalArguments);
	}
	
public:
	void ApplySettings(FRequestPlaySessionParams& Params) const
	{
		Params.bAllowOnlineSubsystem = bUseOnlineSubsystem;
		
		// Set window sized
		Params.EditorPlaySettings->SetClientWindowSize(ClientWindowSize);
		
		// Build out SERVER parameters
		Params.EditorPlaySettings->AdditionalServerLaunchParameters = GetServerArguments();
		
		// Build out CLIENT parameters
		Params.EditorPlaySettings->AdditionalLaunchParameters = GetClientArguments();
		
		// Set the number of clients
		Params.EditorPlaySettings->SetPlayNumberOfClients(ClientCount);
		
		// Setup server mode
		if (bUseDedicatedServer || ServerParameters.Len() > 0)
		{
			Params.EditorPlaySettings->SetPlayNetMode(PIE_Client);
			Params.EditorPlaySettings->bLaunchSeparateServer = bSpawnSeparateServer;
		}
		else
		{
			Params.EditorPlaySettings->SetPlayNetMode(PIE_ListenServer);
			Params.EditorPlaySettings->bLaunchSeparateServer = false;
		}
	}
#endif // WITH_EDITORONLY_DATA
};
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NToolingEditorUserSettings.generated.h"

namespace NEXUS::Tooling::MultiplayerTest
{
	inline const FString Argument = TEXT("NMultiplayerTest");
}

/**
 * Per-user editor preferences for NexusTooling. Stored in NexusUserSettings.ini so each developer
 * keeps their own values (frame-rate cap, visualization colors, graph navigation ergonomics).
 * Property edits re-apply immediately via PostEditChangeProperty.
 */
UCLASS(config = NexusUserSettings, meta = (DisplayName = "Tooling (User)"))
class NEXUSTOOLINGEDITOR_API UNToolingEditorUserSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNToolingEditorUserSettings, "Tooling (User)", "Specific settings for NEXUS: Tooling included with the framework.");

	/** Post-engine-init hook that applies the persisted values to their respective editor systems. */
	static void OnPostEngineInit();

#if WITH_EDITOR
	//~UObject
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//End UObject
#endif // WITH_EDITOR

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, config,  Category="Graph Navigation", DisplayName="Space To Pan", meta=(Tooltip="While holding space down, left click and drag to pan the foreground graph."))
	bool bGraphNavigationSpaceToPan = true;

	UPROPERTY(EditAnywhere, config,  Category="Graph Navigation", DisplayName = "Pan Speed Multiplier", meta=(Tooltip="The multiplier applied when dragging with the Space To Pan feature."))
	float GraphNavigationPanSpeedMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, config, Category = "Editor Performance", DisplayName = "Initial Editor Maximum FPS", meta=(Tooltip="Limit the frame rate of editor, keeping our GPUs quiet and cool. Setting this to 0 will use the default engine value."))
	float EditorFrameRateLimit = 60.0f;

	UPROPERTY(EditAnywhere, config, Category = "Editor Performance", DisplayName = "Always Show Frame Rate & Memory", meta=(Tooltip="Ensures that the checkbox for Show Frame Rate & Memory remains checked, even if local settings are wiped."))
	bool bAlwaysShowFrameRateAndMemory = true;
	
	UPROPERTY(EditAnywhere, config, Category = "Leak Check", DisplayName ="Duration", meta=(ToolTip="How long should the leak check run?"))
	float LeakCheckTime = 30.f;
	
	UPROPERTY(EditAnywhere, config, Category = "Visualizers|Distribution", DisplayName = "Outer Color", meta = (Tooltip = "Color used to draw the outer boundary of a distribution visualizer."))
	FColor VisualizationDistributionOuterColor = FColor(255,0,88);

	UPROPERTY(EditAnywhere, config, Category = "Visualizers|Distribution", DisplayName = "Inner Color", meta = (Tooltip = "Color used to draw the inner boundary of a distribution visualizer."))
	FColor VisualizationDistributionInnerColor = FColor(150,0,89);
	
	UPROPERTY(EditAnywhere, config, Category = "Visualizers|Distribution", DisplayName = "Line Thickness", meta = (Tooltip = "The thickness of debug lines that are drawn."))
	float VisualizationLineThickness = 1.5f;
	
	UPROPERTY(EditAnywhere, config, Category = "Visualizers|Distribution", DisplayName = "Circle Sides",  meta = (Tooltip = "The number of lines used to draw a debug circle."))
	int32 VisualizationCircleSides = 64;

	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Setup", meta = (DisplayName = "Clear Logs Folder", Tooltip = "Clear the log files (not in use) prior to running a test?"))
	bool bClearLogsFolder = false;
	
	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Clients", meta = (DisplayName = "Count", Tooltip = "The number of clients to spawn."))
	int32 ClientCount = 2;

	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Clients", meta = (DisplayName = "Window Size", Tooltip = "The size of the created clients windows."))
	FIntPoint ClientWindowSize = FIntPoint(800, 600);

	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Clients", meta = (DisplayName = "Disable Sound", Tooltip = "Mutes sound of clients."))
	bool bClientDisableSound = false;
	
	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Clients", meta = (DisplayName = "Generate Network Profile", Tooltip = "Profiles will be stored in <PROJECT_DIRECTORY>/Saved/Profiling/<PROJECT_NAME>-<TIMESTAMP>.nprof"))
	bool bClientGenerateNetworkProfile = false;
	
	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Clients", meta = (DisplayName = "Parameters", Tooltip = "Additional parameters to pass to the client being launched."))
	FString ClientParameters = "";
	
	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Clients|Network Simulation", meta = (DisplayName = "Lag (Minimum)", Tooltip = "The minimum amount of network lag (ms) be simulated ontop of the existing round trip time.", ClampMin="0", ClampMax="1000", UIMin="0", UIMax="1000", SliderExponent = 1))
	int32 ClientSimulateLagMinimum = 20;
	
	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Clients|Network Simulation", meta = (DisplayName = "Lag (Maximum)", Tooltip = "The maximum amount of network lag (ms) be simulated ontop of the existing round trip time.", ClampMin="0", ClampMax="1000", UIMin="0", UIMax="1000", SliderExponent = 1))
	int32 ClientSimulateLagMaximum = 60;
	
	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Clients|Network Simulation", meta = (DisplayName = "Packet Loss", Tooltip = "An amount of packet loss (%) to be simulated.", ClampMin="0", ClampMax="100", UIMin="0", UIMax="100", SliderExponent = 1)) 
	int32 ClientSimulatePacketLoss = 0;
	
	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Clients|Network Simulation", meta = (DisplayName = "Packet Jitter", Tooltip = "An amount of packet jitter (ms) to be simulated.", ClampMin="0", ClampMax="1000", UIMin="0", UIMax="1000", SliderExponent = 1)) 
	int32 ClientSimulatePacketJitter = 0;
	
	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Clients|Network Simulation", meta = (DisplayName = "Packet Duplication", Tooltip = "An amount of packet duplication (%) to be simulated.", ClampMin="0", ClampMax="100", UIMin="0", UIMax="100", SliderExponent = 1)) 
	int32 ClientSimulatePacketDuplication = 0;
	
	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Clients|Network Simulation", meta = (DisplayName = "Receive Out Of Order", Tooltip = "Forces network packets to be recieved out of order.")) 
	bool bClientSimulateReceiveOutOfOrderPackets = false;

	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Server", meta = (EditCondition="!bClientGenerateNetworkProfile", EditConditionHides, DisplayName = "Generate Network Profile", Tooltip = "This setting has no effect if option is turned on for clients. Profiles will be stored in <PROJECT_DIRECTORY>/Saved/Profiling/<PROJECT_NAME>-<TIMESTAMP>.nprof"))
	bool bServerGenerateNetworkProfile = false;
	
	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Server", meta = (DisplayName = "Dedicated Server", Tooltip = "Spawn a dedicated server for use by the clients."))
	bool bUseDedicatedServer = true;

	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Server", meta = (DisplayName = "Spawn Separate Server", Tooltip = "This is a rarely used option that will launch a separate server (possibly hidden in-process depending on RunUnderOneProcess) even if the net mode does not require a server (such as Standalone). If the net mode requires a server (such as Client) a server will be launched for you (regardless of this setting)."))
	bool bSpawnSeparateServer = false;

	UPROPERTY(EditAnywhere, config, Category = "Multiplayer Test|Server", meta = (DisplayName = "Parameters", Tooltip = "Additional parameters to pass to the server being launched, on top of client parameters."))
	FString ServerParameters = "";

#endif // WITH_EDITORONLY_DATA

private:
	/** Apply EditorFrameRateLimit to the engine's MaxFPS cvar. */
	void ApplyEditorFrameRateLimit() const;

	/** Force the editor's show-frame-rate-and-memory toggle on when bAlwaysShowFrameRateAndMemory is true. */
	void ApplyAlwaysShowFrameRateAndMemory() const;

	/** Publish bGraphNavigationSpaceToPan and GraphNavigationPanSpeedMultiplier to the shared input processor. */
	void ApplySpaceToPan() const;
	
	/**
	 * Builds the concatenated command-line arguments passed to each client launched by the
	 * multiplayer test session, encoding the simulation toggles plus any user-supplied ClientParameters.
	 * @return A single space-separated argument string prefixed with -NMultiplayerTest.
	 */
	FString GetMultiplayerTestClientArguments() const;
	
	/**
	 * Builds the command-line arguments passed to the server (dedicated or listen) launched
	 * alongside the test session's clients.
	 * @return The concatenated server-side argument string.
	 */
	FString GetMultiplayerTestServerArguments() const;
	
public:
	/**
	 * Copies the user-facing values from this settings object into the supplied FRequestPlaySessionParams
	 * so the editor launches the test session with the configured client/server layout.
	 * @param Params The play-session request to populate; Params.EditorPlaySettings must be valid.
	 */
	void ApplySettings(FRequestPlaySessionParams& Params) const;
};
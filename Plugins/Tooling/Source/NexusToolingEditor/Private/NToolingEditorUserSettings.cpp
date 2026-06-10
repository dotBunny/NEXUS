// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NToolingEditorUserSettings.h"

#include "NEditorInputProcessor.h"
#include "NEditorUtils.h"
#include "NToolingEditorMinimal.h"
#include "NToolingEditorModule.h"
#include "Editor/EditorPerformanceSettings.h"
#include "GameFramework/GameUserSettings.h"

void UNToolingEditorUserSettings::OnPostEngineInit()
{
	if (const UNToolingEditorUserSettings* Settings = Get())
	{
		Settings->ApplyEditorFrameRateLimit();
		Settings->ApplyAlwaysShowFrameRateAndMemory();
		Settings->ApplySpaceToPan();
	}
}

void UNToolingEditorUserSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Check if we are updating the frame rate
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UNToolingEditorUserSettings, EditorFrameRateLimit))
	{
		ApplyEditorFrameRateLimit();
	}

	ApplySpaceToPan();
}

void UNToolingEditorUserSettings::ApplyEditorFrameRateLimit() const
{
	if (FNEditorUtils::IsUserControlled())
	{
		UE_LOG(LogNexusToolingEditor, Log, TEXT("Apply framerate limit(%f)."), EditorFrameRateLimit);

		GEngine->SetMaxFPS(EditorFrameRateLimit);
		if (IsValid(GEngine->GameUserSettings))
		{
			GEngine->GameUserSettings->SetFrameRateLimit(EditorFrameRateLimit);
		}
	}
}

void UNToolingEditorUserSettings::ApplyAlwaysShowFrameRateAndMemory() const
{
	if (FNEditorUtils::IsUserControlled() && bAlwaysShowFrameRateAndMemory)
	{
		if (const UEditorPerformanceSettings* EditorPerformanceSettings = GetDefault<UEditorPerformanceSettings>();
			EditorPerformanceSettings != nullptr &&
			!EditorPerformanceSettings->bShowFrameRateAndMemory)
		{
			UEditorPerformanceSettings* Settings = GetMutableDefault<UEditorPerformanceSettings>();
			Settings->bShowFrameRateAndMemory = 1;
		}
	}
}

void UNToolingEditorUserSettings::ApplySpaceToPan() const
{
	if (FNEditorUtils::IsUserControlled())
	{
		const FNToolingEditorModule& ToolingModule = FModuleManager::GetModuleChecked<FNToolingEditorModule>("NexusToolingEditor");
		FNEditorInputProcessor* InputProcessor = ToolingModule.GetInputProcessor();
		InputProcessor->bCachedGraphNavigationSpaceToPan = bGraphNavigationSpaceToPan;
		InputProcessor->CachedGraphNavigationPanSpeedMultiplier = GraphNavigationPanSpeedMultiplier;
	}
}

FString UNToolingEditorUserSettings::GetMultiplayerTestClientArguments() const
{
	FString ClientArguments = TEXT("-") + NEXUS::Tooling::MultiplayerTest::Argument;
		
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
	
	if (ClientSimulatePacketJitter > 0)
	{
		ClientArguments.Append(FString::Printf(TEXT(" -PktJitter=%i"), ClientSimulatePacketJitter));
	}
		
	if (bClientSimulateReceiveOutOfOrderPackets)
	{
		ClientArguments.Append(" -PktOrder=1");
	}
		
	ClientArguments.Append(FString::Printf(TEXT(" %s"), *ClientParameters.TrimStartAndEnd()));
		
	return ClientArguments;
}

FString UNToolingEditorUserSettings::GetMultiplayerTestServerArguments() const
{
	FString ServerAdditionalArguments = TEXT("");
		
	if (bServerGenerateNetworkProfile)
	{
		// ReSharper disable once StringLiteralTypo
		ServerAdditionalArguments.Append(" networkprofiler=true");
	}
		
	ServerAdditionalArguments.Append(FString::Printf(TEXT(" %s"), *ServerParameters.TrimStartAndEnd()));
		
	return ServerAdditionalArguments;
}

void UNToolingEditorUserSettings::ApplySettings(FRequestPlaySessionParams& Params) const
{
	// Set window sized
	Params.EditorPlaySettings->SetClientWindowSize(ClientWindowSize);
		
	// Build out SERVER parameters
	Params.EditorPlaySettings->AdditionalServerLaunchParameters = GetMultiplayerTestServerArguments();
		
	// Build out CLIENT parameters
	Params.EditorPlaySettings->AdditionalLaunchParameters = GetMultiplayerTestClientArguments();
		
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

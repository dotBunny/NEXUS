// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMultiplayerEditorCommands.h"

#include "NEditorUtils.h"
#include "NMultiplayerEditorModule.h"
#include "NMultiplayerEditorStyle.h"
#include "NMultiplayerEditorUserSettings.h"

bool FNMultiplayerEditorCommands::bIsMultiplayerTestRunning = false;

void FNMultiplayerEditorCommands::Register()
{
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User"))
	{
		FToolMenuSection& NexusSection = Menu->FindOrAddSection("NEXUS_Multiplayer");
		NexusSection.Alignment = EToolMenuSectionAlign::Last;
		NexusSection.AddSeparator(NAME_None);
		
		FToolMenuEntry MultiplayerTest = FToolMenuEntry::InitToolBarButton(
				"Toggle Multiplayer Test",
				FUIAction(
					FExecuteAction::CreateStatic(&ToggleMultiplayerTest),
					FCanExecuteAction::CreateStatic(&FNEditorUtils::IsNotPlayInEditor),
					FIsActionChecked(),
					FIsActionButtonVisible()),
					TAttribute<FText>(),
					TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateStatic(&MultiplayerTest_GetTooltip)),
					TAttribute<FSlateIcon>::Create(TAttribute<FSlateIcon>::FGetter::CreateStatic(&MultiplayerTest_GetIcon)));
		
		NexusSection.AddEntry(MultiplayerTest);
	}
}

FText FNMultiplayerEditorCommands::MultiplayerTest_GetTooltip()
{
	if (bIsMultiplayerTestRunning)
	{
		return NSLOCTEXT("NexusMultiplayerEditor", "Command_StopMultiplayerTest_Tooltip", "Stop the ongoing local multiplayer test");
	}
	return NSLOCTEXT("NexusMultiplayerEditor", "Command_StartMultiplayerTest_Tooltip", "Play this level in a local multiplayer test");
}

FSlateIcon FNMultiplayerEditorCommands::MultiplayerTest_GetIcon()
{
	if (bIsMultiplayerTestRunning)
	{
		return FSlateIcon(FNMultiplayerEditorStyle::GetStyleSetName(), "Command.Multiplayer.StopMultiplayerTest.On");
	}
	return FSlateIcon(FNMultiplayerEditorStyle::GetStyleSetName(), "Command.Multiplayer.StartMultiplayerTest.On");
}

void FNMultiplayerEditorCommands::ToggleMultiplayerTest()
{
	// Get reference to module
	const FNMultiplayerEditorModule& Module = FNMultiplayerEditorModule::Get();
	
	if (bIsMultiplayerTestRunning)
	{
		Module.OnMultiplayerTestEnd.ExecuteIfBound();
		GUnrealEd->EndPlayOnLocalPc();
		bIsMultiplayerTestRunning = false;
	}
	else
	{
		FRequestPlaySessionParams PlaySessionRequest;
		const UNMultiplayerEditorUserSettings* Settings = UNMultiplayerEditorUserSettings::Get();
	
		PlaySessionRequest.bAllowOnlineSubsystem = Settings->bUseOnlineSubsystem;
		PlaySessionRequest.SessionDestination = EPlaySessionDestinationType::NewProcess;

		// Get default editor settings
		PlaySessionRequest.EditorPlaySettings = GetMutableDefault<ULevelEditorPlaySettings>();
		FObjectDuplicationParameters DuplicationParams(PlaySessionRequest.EditorPlaySettings, GetTransientPackage());
		PlaySessionRequest.EditorPlaySettings = CastChecked<ULevelEditorPlaySettings>(StaticDuplicateObjectEx(DuplicationParams));

		// Straight copies
		PlaySessionRequest.EditorPlaySettings->SetClientWindowSize(Settings->ClientWindowSize);
		
		// Build out SERVER parameters
		PlaySessionRequest.EditorPlaySettings->AdditionalServerLaunchParameters = Settings->ServerParameters;
		if (Settings->bServerGenerateNetworkProfile)
		{
			// ReSharper disable once StringLiteralTypo
			PlaySessionRequest.EditorPlaySettings->AdditionalServerLaunchParameters.Append(" networkprofiler=true");
		}
		
		// Add blanket argument that will be captured and used to tell that a test is running
		PlaySessionRequest.EditorPlaySettings->AdditionalServerLaunchParameters.Append(" -NMultiplayerTest");

		// Build out CLIENT parameters
		PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters = Settings->ClientParameters;
		
		if (Settings->bClientGenerateNetworkProfile)
		{
			// ReSharper disable once StringLiteralTypo
			PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters.Append(" networkprofiler=true");
		}
		
		// Add blanket argument that will be captured and used to tell that a test is running
		PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters.Append(" -NMultiplayerTest");
		
		if (Settings->bClientDisableSound)
		{
			// ReSharper disable once StringLiteralTypo
			PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters.Append(" -nosound");
		}
		if (Settings->ClientSimulateLagMinimum > 0)
		{
			PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters.Append(
				FString::Printf(TEXT(" -PktLagMin=%i"), FMath::FloorToInt(Settings->ClientSimulateLagMinimum * 0.5f)));
		}
		else if (Settings->ClientSimulateLagMaximum > 0)
		{
			PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters.Append(" -PktLagMin=0");
		}
		
		if (Settings->ClientSimulateLagMaximum > 0)
		{
			PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters.Append(
				FString::Printf(TEXT(" -PktLagMax=%i"), FMath::FloorToInt(Settings->ClientSimulateLagMaximum * 0.5f)));
		}
		
		if (Settings->ClientSimulatePacketLoss > 0)
		{
			PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters.Append(
				FString::Printf(TEXT(" -PktLoss=%i"), Settings->ClientSimulatePacketLoss));
		}
		
		if (Settings->ClientSimulatePacketDuplication > 0)
		{
			PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters.Append(
				FString::Printf(TEXT(" -PktDup=%i"), Settings->ClientSimulatePacketDuplication));
		}
		
		if (Settings->bClientSimulateReceiveOutOfOrderPackets)
		{
			PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters.Append(" -PktOrder=1");
		}
		
		//PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters.Append(" -=1");
		
		
		
		// Are there any additional parameters that a binding has provided?
		if (Module.OnMultiplayerTestStart.IsBound())
		{
			const FString AdditionalArgs = Module.OnMultiplayerTestStart.Execute().TrimStartAndEnd();
			if (!AdditionalArgs.IsEmpty())
			{
				PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters.Append(
					FString::Printf(TEXT(" %s"), *AdditionalArgs));
			}
		}

		PlaySessionRequest.EditorPlaySettings->SetRunUnderOneProcess(false);
		PlaySessionRequest.EditorPlaySettings->SetPlayNumberOfClients(Settings->ClientCount);

		if (Settings->bUseDedicatedServer || Settings->ServerParameters.Len() > 0)
		{
			PlaySessionRequest.EditorPlaySettings->SetPlayNetMode(PIE_Client);
			PlaySessionRequest.EditorPlaySettings->bLaunchSeparateServer = Settings->bSpawnSeparateServer;
		}
		else
		{
			PlaySessionRequest.EditorPlaySettings->SetPlayNetMode(PIE_ListenServer);
			PlaySessionRequest.EditorPlaySettings->bLaunchSeparateServer = false;
		}

		// Start!
		GUnrealEd->RequestPlaySession(PlaySessionRequest);

		// Toggle our tracker
		bIsMultiplayerTestRunning = true;
	}
}
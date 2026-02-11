// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMultiplayerEditorCommands.h"
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
	if (bIsMultiplayerTestRunning)
	{
		GUnrealEd->EndPlayOnLocalPc();
		bIsMultiplayerTestRunning = false;
	}
	else
	{
		FRequestPlaySessionParams PlaySessionRequest;
		const UNMultiplayerEditorUserSettings* Settings = UNMultiplayerEditorUserSettings::Get();
		const FString MultiplayerFlag = TEXT(" -NMultiplayerTest");
		// ReSharper disable once StringLiteralTypo
		const FString NetworkProfileFlag = TEXT(" networkprofiler=true");
	
		PlaySessionRequest.bAllowOnlineSubsystem = Settings->bUseOnlineSubsystem;
		PlaySessionRequest.SessionDestination = EPlaySessionDestinationType::NewProcess;

		// Get default editor settings
		PlaySessionRequest.EditorPlaySettings = GetMutableDefault<ULevelEditorPlaySettings>();
		FObjectDuplicationParameters DuplicationParams(PlaySessionRequest.EditorPlaySettings, GetTransientPackage());
		PlaySessionRequest.EditorPlaySettings = CastChecked<ULevelEditorPlaySettings>(StaticDuplicateObjectEx(DuplicationParams));

		// Straight copies
		PlaySessionRequest.EditorPlaySettings->SetClientWindowSize(Settings->ClientWindowSize);
		
		// Build out Server parameters
		PlaySessionRequest.EditorPlaySettings->AdditionalServerLaunchParameters = Settings->ServerParameters;
		if (Settings->bServerGenerateNetworkProfile)
		{
			PlaySessionRequest.EditorPlaySettings->AdditionalServerLaunchParameters.Append(NetworkProfileFlag);
		}
		PlaySessionRequest.EditorPlaySettings->AdditionalServerLaunchParameters.Append(MultiplayerFlag);

		// Build out Client parameters
		PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters = Settings->ClientParameters;
		if (Settings->bClientGenerateNetworkProfile)
		{
			PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters.Append(NetworkProfileFlag);
		}
		PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters.Append(MultiplayerFlag);
		
		PlaySessionRequest.EditorPlaySettings->SetRunUnderOneProcess(false);
		PlaySessionRequest.EditorPlaySettings->SetPlayNumberOfClients(Settings->ClientCount);

		if (Settings->bUseDedicatedServer)
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
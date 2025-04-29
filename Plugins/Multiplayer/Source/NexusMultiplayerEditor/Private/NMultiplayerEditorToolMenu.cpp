// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMultiplayerEditorToolMenu.h"
#include "NMultiplayerEditorStyle.h"
#include "NMultiplayerEditorUserSettings.h"

#define LOCTEXT_NAMESPACE "NexusMultiplayerEditor"

bool FNMultiplayerEditorToolMenu::bIsMultiplayerTestRunning = false;

void FNMultiplayerEditorToolMenu::Register()
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
					FCanExecuteAction::CreateStatic(&MultiplayerTest_CanExecute),
					FIsActionChecked(),
					FIsActionButtonVisible()),
					TAttribute<FText>(),
					TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateStatic(&MultiplayerTest_GetTooltip)),
					TAttribute<FSlateIcon>::Create(TAttribute<FSlateIcon>::FGetter::CreateStatic(&MultiplayerTest_GetIcon)));
		
		NexusSection.AddEntry(MultiplayerTest);
	}
}

FText FNMultiplayerEditorToolMenu::MultiplayerTest_GetTooltip()
{
	if (bIsMultiplayerTestRunning)
	{
		return LOCTEXT("Command_NMultiplayer_StopMultiplayerTest_Tooltip", "Stop the ongoing local multiplayer test");
	}
	return LOCTEXT("Command_NMultiplayer_StartMultiplayerTest_Tooltip", "Play this level in a local multiplayer test");
}

FSlateIcon FNMultiplayerEditorToolMenu::MultiplayerTest_GetIcon()
{
	if (bIsMultiplayerTestRunning)
	{
		return FSlateIcon(FNMultiplayerEditorStyle::GetStyleSetName(), "Command.Multiplayer.StopMultiplayerTest.On");
	}
	return FSlateIcon(FNMultiplayerEditorStyle::GetStyleSetName(), "Command.Multiplayer.StartMultiplayerTest.On");
}

bool FNMultiplayerEditorToolMenu::MultiplayerTest_CanExecute()
{
	return FNEditorUtils::IsNotPlayInEditor();
}

void FNMultiplayerEditorToolMenu::ToggleMultiplayerTest()
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
	
		PlaySessionRequest.bAllowOnlineSubsystem = Settings->bUseOnlineSubsystem;
		PlaySessionRequest.SessionDestination = EPlaySessionDestinationType::NewProcess;

		// Get default editor settings
		PlaySessionRequest.EditorPlaySettings = GetMutableDefault<ULevelEditorPlaySettings>();
		FObjectDuplicationParameters DuplicationParams(PlaySessionRequest.EditorPlaySettings, GetTransientPackage());
		PlaySessionRequest.EditorPlaySettings = CastChecked<ULevelEditorPlaySettings>(StaticDuplicateObjectEx(DuplicationParams));

		// Overrides
		PlaySessionRequest.EditorPlaySettings->AdditionalServerLaunchParameters = Settings->ServerParameters;
		PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters = Settings->ClientParameters;
		
		
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

#undef LOCTEXT_NAMESPACE
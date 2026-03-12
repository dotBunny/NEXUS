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
		PlaySessionRequest.SessionDestination = EPlaySessionDestinationType::NewProcess;

		// Get default editor settings
		PlaySessionRequest.EditorPlaySettings = GetMutableDefault<ULevelEditorPlaySettings>();
		FObjectDuplicationParameters DuplicationParams(PlaySessionRequest.EditorPlaySettings, GetTransientPackage());
		PlaySessionRequest.EditorPlaySettings = CastChecked<ULevelEditorPlaySettings>(StaticDuplicateObjectEx(DuplicationParams));

#if WITH_EDITORONLY_DATA
		
		const UNMultiplayerEditorUserSettings* Settings = UNMultiplayerEditorUserSettings::Get();
		Settings->ApplySettings(PlaySessionRequest);
		
#endif // WITH_EDITORONLY_DATA
		
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

		// Start!
		GUnrealEd->RequestPlaySession(PlaySessionRequest);

		// Toggle our tracker
		bIsMultiplayerTestRunning = true;
	}
}
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMultiplayerEditorCommands.h"

#include "NEditorUtils.h"
#include "NMultiplayerEditorModule.h"
#include "NMultiplayerEditorStyle.h"
#include "NMultiplayerEditorSubsystem.h"

bool FNMultiplayerEditorCommands::bIsTestRunning = false;
FDelegateHandle FNMultiplayerEditorCommands::OnTestStartedHandle;
FDelegateHandle FNMultiplayerEditorCommands::OnTestEndedHandle;

bool FNMultiplayerEditorCommands::MultiplayerTest_CanExecute()
{
	return FNEditorUtils::IsNotPlayInEditor();
}

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
					FCanExecuteAction::CreateStatic(&MultiplayerTest_CanExecute),
					FIsActionChecked(),
					FIsActionButtonVisible()),
					TAttribute<FText>(),
					TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateStatic(&MultiplayerTest_GetTooltip)),
					TAttribute<FSlateIcon>::Create(TAttribute<FSlateIcon>::FGetter::CreateStatic(&MultiplayerTest_GetIcon)));
		
		NexusSection.AddEntry(MultiplayerTest);
		
		FNMultiplayerEditorModule& Module = FNMultiplayerEditorModule::Get();
		OnTestStartedHandle = Module.OnMultiplayerTestStarted.AddStatic(OnMultiplayerTestStarted);
		OnTestEndedHandle = Module.OnMultiplayerTestEnded.AddStatic(OnMultiplayerTestEnded);
	}
}

void FNMultiplayerEditorCommands::Unregister()
{
	FNMultiplayerEditorModule& Module = FNMultiplayerEditorModule::Get();

	if (OnTestStartedHandle.IsValid())
	{
		Module.OnMultiplayerTestStarted.Remove(OnTestStartedHandle);
	}
	
	if (OnTestEndedHandle.IsValid())
	{
		Module.OnMultiplayerTestEnded.Remove(OnTestEndedHandle);
	}
}

FText FNMultiplayerEditorCommands::MultiplayerTest_GetTooltip()
{
	if (bIsTestRunning)
	{
		return NSLOCTEXT("NexusMultiplayerEditor", "Command_StopMultiplayerTest_Tooltip", "Stop the ongoing local multiplayer test");
	}
	return NSLOCTEXT("NexusMultiplayerEditor", "Command_StartMultiplayerTest_Tooltip", "Play this level in a local multiplayer test");
}

FSlateIcon FNMultiplayerEditorCommands::MultiplayerTest_GetIcon()
{
	if (bIsTestRunning)
	{
		return FSlateIcon(FNMultiplayerEditorStyle::GetStyleSetName(), "Command.Multiplayer.StopMultiplayerTest.On");
	}
	return FSlateIcon(FNMultiplayerEditorStyle::GetStyleSetName(), "Command.Multiplayer.StartMultiplayerTest.On");
}

void FNMultiplayerEditorCommands::ToggleMultiplayerTest()
{
	UNMultiplayerEditorSubsystem::Get()->ToggleMultiplayerTest();
}

void FNMultiplayerEditorCommands::OnMultiplayerTestStarted()
{
	bIsTestRunning = true;
}

void FNMultiplayerEditorCommands::OnMultiplayerTestEnded()
{
	bIsTestRunning = false;
}

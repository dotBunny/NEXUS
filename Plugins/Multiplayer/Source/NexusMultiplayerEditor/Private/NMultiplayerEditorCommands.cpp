// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMultiplayerEditorCommands.h"

#include "NCoreEditorMinimal.h"
#include "NEditorUtils.h"
#include "NMultiplayerEditorModule.h"
#include "NMultiplayerEditorSettings.h"
#include "NMultiplayerEditorStyle.h"
#include "NMultiplayerEditorSubsystem.h"
#include "ShaderCompiler.h"
#include "ToolMenu.h"

bool FNMultiplayerEditorCommands::bIsTestRunning = false;
FDelegateHandle FNMultiplayerEditorCommands::OnTestStartedHandle;
FDelegateHandle FNMultiplayerEditorCommands::OnTestEndedHandle;
FName FNMultiplayerEditorCommands::MultiplayerTestSectionName = FName("NEXUS_Multiplayer");

bool FNMultiplayerEditorCommands::MultiplayerTest_CanExecute()
{
	return FNEditorUtils::IsNotPlayInEditor() && !IsAsyncLoading() && (GShaderCompilingManager ? !GShaderCompilingManager->IsCompiling() : true);
}

void FNMultiplayerEditorCommands::Register()
{
	const UNMultiplayerEditorSettings* Settings = UNMultiplayerEditorSettings::Get();
	
	if (!Settings->bMultiplayerTestEnabled) return;
	AddMultiplayerTestSection();
}

void FNMultiplayerEditorCommands::Unregister()
{
	if (HasMultiplayerTestSection())
	{
		RemoveMultiplayerTestSection();
	}
}

void FNMultiplayerEditorCommands::AddMultiplayerTestSection()
{
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser))
	{
		FToolMenuSection& MultiplayerSection = Menu->FindOrAddSection(MultiplayerTestSectionName);
		MultiplayerSection.Alignment = EToolMenuSectionAlign::Last;
		MultiplayerSection.AddSeparator(NAME_None);
		
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
		
		MultiplayerSection.AddEntry(MultiplayerTest);
		
		FNMultiplayerEditorModule& Module = FNMultiplayerEditorModule::Get();
		OnTestStartedHandle = Module.OnMultiplayerTestStarted.AddStatic(OnMultiplayerTestStarted);
		OnTestEndedHandle = Module.OnMultiplayerTestEnded.AddStatic(OnMultiplayerTestEnded);
	}
}

void FNMultiplayerEditorCommands::RemoveMultiplayerTestSection()
{
	UToolMenus* ToolMenus = UToolMenus::TryGet();
	if (ToolMenus != nullptr)
	{
		if (UToolMenu* Menu = ToolMenus->FindMenu(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser))
		{
			Menu->RemoveSection(MultiplayerTestSectionName);
			ToolMenus->RefreshMenuWidget(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser);
		}
	}
	
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

bool FNMultiplayerEditorCommands::HasMultiplayerTestSection()
{
	// Is shutting down?
	UToolMenus* ToolMenus = UToolMenus::TryGet();
	if (ToolMenus == nullptr) return false;
	if (UToolMenu* Menu = ToolMenus->FindMenu(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser))
	{
		return Menu->FindSection(MultiplayerTestSectionName) != nullptr;
	}
	return false;
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

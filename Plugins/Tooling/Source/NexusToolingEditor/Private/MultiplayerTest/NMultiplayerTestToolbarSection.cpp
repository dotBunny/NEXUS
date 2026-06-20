// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "MultiplayerTest/NMultiplayerTestToolbarSection.h"

#include "NCoreEditorMinimal.h"
#include "NEditorUtils.h"
#include "MultiplayerTest/NMultiplayerTestSubsystem.h"
#include "NToolingEditorModule.h"
#include "NToolingEditorSettings.h"
#include "NToolingEditorStyle.h"
#include "ShaderCompiler.h"
#include "ToolMenu.h"

bool FNMultiplayerTestToolbarSection::bIsTestRunning = false;
FDelegateHandle FNMultiplayerTestToolbarSection::OnTestStartedHandle;
FDelegateHandle FNMultiplayerTestToolbarSection::OnTestEndedHandle;
FName FNMultiplayerTestToolbarSection::MultiplayerTestSectionName = FName("NEXUS_Multiplayer");

bool FNMultiplayerTestToolbarSection::MultiplayerTest_CanExecute()
{
	return FNEditorUtils::IsNotPlayInEditor() && !IsAsyncLoading() && (GShaderCompilingManager ? !GShaderCompilingManager->IsCompiling() : true);
}

void FNMultiplayerTestToolbarSection::AddSection()
{
	const UNToolingEditorSettings* Settings = UNToolingEditorSettings::Get();

	if (!Settings->bMultiplayerTestEnabled) return;
	AddMenuEntry();
}

void FNMultiplayerTestToolbarSection::RemoveSection()
{
	if (HasSection())
	{
		RemoveMenuEntry();
	}
}

void FNMultiplayerTestToolbarSection::AddMenuEntry()
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

		FNToolingEditorModule& Module = FNToolingEditorModule::Get();
		OnTestStartedHandle = Module.OnMultiplayerTestStarted.AddStatic(OnMultiplayerTestStarted);
		OnTestEndedHandle = Module.OnMultiplayerTestEnded.AddStatic(OnMultiplayerTestEnded);
	}
}

void FNMultiplayerTestToolbarSection::RemoveMenuEntry()
{
	UToolMenus* ToolMenus = UToolMenus::TryGet();
	if (ToolMenus != nullptr)
	{
		UToolMenu* Menu = ToolMenus->FindMenu(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser);
		if (Menu != nullptr)
		{
			Menu->RemoveSection(MultiplayerTestSectionName);
			ToolMenus->RefreshMenuWidget(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser);
		}
	}

	FNToolingEditorModule& Module = FNToolingEditorModule::Get();

	if (OnTestStartedHandle.IsValid())
	{
		Module.OnMultiplayerTestStarted.Remove(OnTestStartedHandle);
	}

	if (OnTestEndedHandle.IsValid())
	{
		Module.OnMultiplayerTestEnded.Remove(OnTestEndedHandle);
	}
}

bool FNMultiplayerTestToolbarSection::HasSection()
{
	// Is shutting down?
	UToolMenus* ToolMenus = UToolMenus::TryGet();
	if (ToolMenus == nullptr) return false;
	UToolMenu* Menu = ToolMenus->FindMenu(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser);
	if (Menu != nullptr)
	{
		return Menu->FindSection(MultiplayerTestSectionName) != nullptr;
	}
	return false;
}

FText FNMultiplayerTestToolbarSection::MultiplayerTest_GetTooltip()
{
	if (bIsTestRunning)
	{
		return NSLOCTEXT("NexusMultiplayerEditor", "Command_StopMultiplayerTest_Tooltip", "Stop the ongoing local multiplayer test");
	}
	return NSLOCTEXT("NexusMultiplayerEditor", "Command_StartMultiplayerTest_Tooltip", "Play this level in a local multiplayer test");
}

FSlateIcon FNMultiplayerTestToolbarSection::MultiplayerTest_GetIcon()
{
	if (bIsTestRunning)
	{
		return FSlateIcon(FNToolingEditorStyle::GetStyleSetName(), "Command.Multiplayer.StopMultiplayerTest.On");
	}
	return FSlateIcon(FNToolingEditorStyle::GetStyleSetName(), "Command.Multiplayer.StartMultiplayerTest.On");
}

void FNMultiplayerTestToolbarSection::ToggleMultiplayerTest()
{
	UNMultiplayerTestSubsystem::Get()->ToggleMultiplayerTest();
}

void FNMultiplayerTestToolbarSection::OnMultiplayerTestStarted()
{
	bIsTestRunning = true;
}

void FNMultiplayerTestToolbarSection::OnMultiplayerTestEnded()
{
	bIsTestRunning = false;
}

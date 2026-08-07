// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Commands/NWorldAssemblyEditorOrganCommands.h"

#include "NEditorUtils.h"
#include "NWorldAssemblyEditorStyle.h"
#include "NWorldAssemblyEditorUtils.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "Operations/NWorldAssemblyEditorOrganOperations.h"

FNWorldAssemblyEditorOrganCommands& FNWorldAssemblyEditorOrganCommands::Get()
{
	static FNWorldAssemblyEditorOrganCommands Instance;
	return Instance;
}

void FNWorldAssemblyEditorOrganCommands::Register(const TSharedRef<FBindingContext>& Context)
{
	FNWorldAssemblyEditorOrganCommands& Commands = Get();

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_AddVolume,
		"NWorldAssembly.NOrganComponent.AddVolume",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_AddVolume", "Add Organ Volume"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_AddVolume_Tooltip", "Place a new Organ Volume in the current level, which bounds where an assembly operation may generate."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "ClassIcon.NOrganVolume"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_GenerateProxies,
		"NWorldAssembly.NOrganComponent.GenerateProxies",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_GenerateProxies", "Generate"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_GenerateProxies_Tooltip", "Dispatches an NWorldAssemblyOperation via the UNWorldAssemblyEditorSubsystem to generate the selected UNOrganComponents output NCellProxy actors."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellProxy"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_GenerateAllProxies,
		"NWorldAssembly.NOrganComponent.GenerateAllProxies",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_GenerateAllProxies", "Generate All Proxies"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_GenerateAllProxies_Tooltip", "Dispatches an NWorldAssemblyOperation via the UNWorldAssemblyEditorSubsystem to generate all UNOrganComponents in the world outputing NCellProxy actors."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellProxy"),
		EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::Home));

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_CreateLevelInstances,
		"NWorldAssembly.NOrganComponent.LoadProxies",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_LoadProxies", "Create"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_LoadProxies_Tooltip", "Load the level instance from the selected proxies."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellLevelInstance"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_ClearProxies,
		"NWorldAssembly.NOrganComponent.ClearProxies",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_ClearProxies", "Clear"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_ClearProxies_Tooltip", "Remove generated NCellProxy actors from the world for the selected components operations."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellProxy"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_ClearAllProxies,
		"NWorldAssembly.NOrganComponent.ClearAllProxies",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_ClearAllProxies", "Clear All Proxies"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_ClearAllProxies_Tooltip", "Remove all generated NCellProxy actors from the world."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellProxy"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_CreateAllLevelInstances,
		"NWorldAssembly.NOrganComponent.LoadAllProxies",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_LoadAllProxies", "Create & Load All Level Instances"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_LoadAllProxies_Tooltip", "Creates and then loads all level instances."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellLevelInstance"),
		EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::End));

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_UnloadLevelInstances,
		"NWorldAssembly.NOrganComponent.UnloadProxies",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_UnloadProxies", "Unload"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_UnloadProxies_Tooltip", "Unload the level instances from the selected proxies"),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellLevelInstance"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_UnloadAllLevelInstances,
		"NWorldAssembly.NOrganComponent.UnloadAllProxies",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_UnloadAllProxies", "Unload All Level Instances"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_UnloadAllProxies_Tooltip", "Unload all level instances."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellLevelInstance"),
		EUserInterfaceActionType::Button, FInputChord());

	Commands.CommandList = MakeShared<FUICommandList>();
	for (const FNCommandInfoAction& Action : GetActions())
	{
		Commands.CommandList->MapAction(Action.CommandInfo, Action.Execute, Action.CanExecute);
	}
}

TArray<FNWorldAssemblyEditorOrganCommands::FNCommandInfoAction> FNWorldAssemblyEditorOrganCommands::GetActions()
{
	using FOperations = FNWorldAssemblyEditorOrganOperations;
	const FNWorldAssemblyEditorOrganCommands& Commands = Get();

	return {
		{ Commands.CommandInfo_AddVolume,               FExecuteAction::CreateStatic(&FOperations::AddVolume),            FCanExecuteAction::CreateStatic(&FNEditorUtils::IsNotPlayInEditor) },
		{ Commands.CommandInfo_GenerateProxies,         FExecuteAction::CreateStatic(&FOperations::GenerateProxies),      FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::CanGenerateSelectedOrgan) },
		{ Commands.CommandInfo_GenerateAllProxies,      FExecuteAction::CreateStatic(&FOperations::GenerateAllProxies),   FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::CanGenerateAllOrgans) },
		{ Commands.CommandInfo_ClearProxies,            FExecuteAction::CreateStatic(&FOperations::ClearGenerated),       FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::HasSelectedGeneratedCellProxies) },
		{ Commands.CommandInfo_ClearAllProxies,         FExecuteAction::CreateStatic(&FOperations::ClearAllProxies),      FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::HasGeneratedCellProxies) },
		{ Commands.CommandInfo_CreateLevelInstances,    FExecuteAction::CreateStatic(&FOperations::LoadProxyLevels),      FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::HasSelectedGeneratedCellProxies) },
		{ Commands.CommandInfo_CreateAllLevelInstances, FExecuteAction::CreateStatic(&FOperations::LoadAllProxyLevels),   FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::HasGeneratedCellProxies) },
		{ Commands.CommandInfo_UnloadLevelInstances,    FExecuteAction::CreateStatic(&FOperations::UnloadProxyLevels),    FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::HasSelectedGeneratedCellProxies) },
		{ Commands.CommandInfo_UnloadAllLevelInstances, FExecuteAction::CreateStatic(&FOperations::UnloadAllProxyLevels), FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::HasGeneratedCellProxies) },
	};
}

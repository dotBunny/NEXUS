// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorCommands.h"

#include "AssetViewUtils.h"
#include "AssetDefinitions/AssetDefinition_NCell.h"
#include "EditorAssetLibrary.h"
#include "FileHelpers.h"
#include "LevelEditor.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "NWorldAssemblyEditorSettings.h"
#include "NWorldAssemblyEditorSubsystem.h"
#include "NWorldAssemblyEditorUserSettings.h"
#include "NWorldAssemblyEditorUtils.h"
#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblyUtils.h"
#include "NUIEditorStyle.h"
#include "NWorldAssemblyEditorToolMenu.h"
#include "Selection.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Assembly/NAssemblyOperation.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Types/NRawMeshFactory.h"

void FNWorldAssemblyEditorCommands::RegisterCommands()
{
	// Build NCell Command Info
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellCaptureThumbnail,
			"NWorldAssembly.NCell.CaptureThumbnail",
			NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CaptureThumbnail", "Capture Thumbnail"),
			NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CaptureThumbnail_Tooltip", "Captures the active viewport (minus widgets) as the thumbnail for the level containing the NCell."),
			FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
			EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellCalculateAll,
		"NWorldAssembly.NCell.CalculateAll",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateAll", "Calculate All"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateAll_Tooltip", "Calculate all data related to the cell."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellCalculateBounds,
		"NWorldAssembly.NCell.CalculateBounds",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateBounds", "Calculate Bounds"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateBounds_Tooltip", "Calculate bounds for the cell."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.CalculateBounds"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellCalculateHull,
		"NWorldAssembly.NCell.CalculateHull",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateHull", "Calculate Hull"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateHull_Tooltip", "Calculate convex hull for the cell."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.CalculateHull"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellCalculateVoxelData,
		"NWorldAssembly.NCell.CalculateVoxelData",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateVoxelData", "Calculate Voxel Data"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateVoxelData_Tooltip", "Calculate voxel data for the cell."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.CalculateVoxelData"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellToggleBoundsCalculateOnSave,
		"NWorldAssembly.NCell.ToggleBoundsCalculateOnSave",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleBoundsCalculateOnSave", "Calculate Bounds On Save"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleBoundsCalculateOnSave_Tooltip", "Calculates the bounds of the cell when the level is saved."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.CalculateBounds"),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellToggleHullAllowNonConvex,
	"NWorldAssembly.NCell.ToggleHullAllowNonConvex",
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleHullAllowNonConvex", "Allow Non-Convex Hull"),
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleHullAllowNonConvex_Tooltip", "Allows for more complex collision mesh to be used instead of optimized convex hull."),
	FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.Hull.AllowNonConvex"),
	EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellToggleHullCalculateOnSave,
		"NWorldAssembly.NCell.ToggleHullCalculateOnSave",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleHullCalculateOnSave", "Calculate Hull On Save"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleHullCalculateOnSave_Tooltip", "Calculates the hull of the cell when the level is saved."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.CalculateHull"),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellToggleVoxelCalculateOnSave,
		"NWorldAssembly.NCell.ToggleVoxelCalculateOnSave",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleVoxelCalculateOnSave", "Calculate Voxel Data On Save"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleVoxelCalculateOnSave_Tooltip", "Calculates the voxel data of the cell when the level is saved."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.CalculateVoxelData"),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellToggleVoxelData,
		"NWorldAssembly.NCell.ToggleVoxelData",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleVoxelData", "Use Voxel Data w/ Cell"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleVoxelData_Tooltip", "Should voxel data be generated and associated to this cell?"),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.Voxel.Grid.Selected"),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellResetCell,
		"NWorldAssembly.NCell.ResetCell",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ResetCell", "Reset Cell"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ResetCell_Tooltip", "Reset the cell data."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Reset"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellSaveCell,
		"NWorldAssembly.NCell.SaveCell",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_SaveCell", "Save Cell"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_SaveCell_Tooltip", "Forcibly write out the Cells data to the sidecar file."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Save"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellRemoveActor,
		"NWorldAssembly.NCell.RemoveActor",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_RemoveActor", "Remove Actor"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_RemoveActor_Tooltip", "Removes the cell actor, no longer making this a cell."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.RemoveNCellActor"),
		EUserInterfaceActionType::Button, FInputChord());

	// Create NCell Command List
	CommandList_Cell = MakeShared<FUICommandList>();

	// Map NCell Actions
	CommandList_Cell->MapAction(Get().CommandInfo_CellCaptureThumbnail,
		FExecuteAction::CreateStatic(&CellCaptureThumbnail),
		FCanExecuteAction::CreateStatic(&CellCaptureThumbnail_CanExecute));

	CommandList_Cell->MapAction(Get().CommandInfo_CellCalculateAll,
		FExecuteAction::CreateStatic(&CellCalculateAll),
		FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld));

	CommandList_Cell->MapAction(Get().CommandInfo_CellCalculateBounds,
		FExecuteAction::CreateStatic(&CellCalculateBounds),
		FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld));

	CommandList_Cell->MapAction(Get().CommandInfo_CellCalculateHull,
	FExecuteAction::CreateStatic(&CellCalculateHull),
	FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld));

	CommandList_Cell->MapAction(Get().CommandInfo_CellCalculateVoxelData,
FExecuteAction::CreateStatic(&CellCalculateVoxelData),
	FCanExecuteAction::CreateStatic(&CellCalculateVoxelData_CanExecute));

	CommandList_Cell->MapAction(Get().CommandInfo_CellToggleBoundsCalculateOnSave,
FExecuteAction::CreateStatic(&CellToggleBoundsCalculateOnSave),
	FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld),
	FIsActionChecked::CreateStatic(&CellToggleBoundsCalculateOnSave_IsActionChecked));

	CommandList_Cell->MapAction(Get().CommandInfo_CellToggleHullCalculateOnSave,
FExecuteAction::CreateStatic(&CellToggleHullCalculateOnSave),
	FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld),
	FIsActionChecked::CreateStatic(&CellToggleHullCalculateOnSave_IsActionChecked));

	CommandList_Cell->MapAction(Get().CommandInfo_CellToggleHullAllowNonConvex,
FExecuteAction::CreateStatic(&CellToggleHullAllowNonConvex),
	FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld),
	FIsActionChecked::CreateStatic(&CellToggleHullAllowNonConvex_IsActionChecked));

	CommandList_Cell->MapAction(Get().CommandInfo_CellToggleVoxelCalculateOnSave,
FExecuteAction::CreateStatic(&CellToggleVoxelCalculateOnSave),
	FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld),
	FIsActionChecked::CreateStatic(&CellToggleVoxelCalculateOnSave_IsActionChecked));

	CommandList_Cell->MapAction(Get().CommandInfo_CellToggleVoxelData,
FExecuteAction::CreateStatic(&CellToggleVoxelData),
	FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld),
	FIsActionChecked::CreateStatic(&CellToggleVoxelData_IsActionChecked));

	CommandList_Cell->MapAction(Get().CommandInfo_CellResetCell,
	FExecuteAction::CreateStatic(&CellResetCell),
	FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld));

	CommandList_Cell->MapAction(Get().CommandInfo_CellSaveCell,
	FExecuteAction::CreateStatic(&CellSaveCell),
	FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld));

	CommandList_Cell->MapAction(Get().CommandInfo_CellRemoveActor,
	FExecuteAction::CreateStatic(&CellRemoveActor),
	FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld));

	// Build NCellJunction Command Info
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellJunctionAddComponent,
		"NWorldAssembly.NCellJunction.AddComponent",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_AddComponent", "Add Component"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_AddComponent_Tooltip", "Add a NCellJunction component to current actor."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellJunctionSelectComponent,
	"NWorldAssembly.NCellJunction.SelectComponent",
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_SelectComponent", "Select Component"),
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_SelectComponent_Tooltip", "Select a NCellJunction in the level."),
	FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
	EUserInterfaceActionType::Button, FInputChord());

	// Collect Components
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellJunctionCollectComponents,
		"NWorldAssembly.NCellJunction.CollectComponents",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_CollectComponents", "Collect Components"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_CollectComponents_Tooltip", "Collects all Junctions and move them to the selected Actor, maintaining their world transforms."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.Junction.CollectJunctionComponents"),
		EUserInterfaceActionType::Button, FInputChord());

	// Create NCell Command List
	CommandList_CellJunction = MakeShared<FUICommandList>();

	CommandList_CellJunction->MapAction(Get().CommandInfo_CellJunctionAddComponent,
FExecuteAction::CreateStatic(&CellJunctionAddComponent),
	FCanExecuteAction::CreateStatic(&FNEditorUtils::HasActorsSelected));

	CommandList_CellJunction->MapAction(Get().CommandInfo_CellJunctionCollectComponents,
FExecuteAction::CreateStatic(&CellJunctionCollectComponents),
	FCanExecuteAction::CreateStatic(&FNEditorUtils::HasActorSelected));


	// Organ
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_OrganGenerateProxies,
	"NWorldAssembly.NOrganComponent.GenerateProxies",
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_GenerateProxies", "Generate Proxies"),
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_GenerateProxies_Tooltip", "Dispatches an NWorldAssemblyOperation via the UNWorldAssemblyEditorSubsystem to generate the selected UNOrganComponents output NCellProxy actors."),
	FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellProxy"),
	EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_OrganGenerateAllProxies,
	"NWorldAssembly.NOrganComponent.GenerateAllProxies",
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_GenerateAllProxies", "Generate All Proxies"),
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_GenerateAllProxies_Tooltip", "Dispatches an NWorldAssemblyOperation via the UNWorldAssemblyEditorSubsystem to generate all UNOrganComponents in the world outputing NCellProxy actors."),
	FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellProxy"),
	EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::Home));

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_OrganCreateLevelInstances,
	"NWorldAssembly.NOrganComponent.LoadProxies",
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_LoadProxies", "Create Level Instances"),
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_LoadProxies_Tooltip", "Load the level instance from the selected proxies."),
	FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellLevelInstance"),
	EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_OrganClearProxies,
	"NWorldAssembly.NOrganComponent.ClearProxies",
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_ClearProxies", "Clear Proxies"),
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_ClearProxies_Tooltip", "Remove generated NCellProxy actors from the world for the selected components operations."),
	FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellProxy"),
	EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_OrganClearAllProxies,
	"NWorldAssembly.NOrganComponent.ClearAllProxies",
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_ClearAllProxies", "Clear All Proxies"),
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_ClearAllProxies_Tooltip", "Remove all generated NCellProxy actors from the world."),
	FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellProxy"),
	EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_OrganCreateAllLevelInstances,
	"NWorldAssembly.NOrganComponent.LoadAllProxies",
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_LoadAllProxies", "Create & Load All Level Instances"),
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_LoadAllProxies_Tooltip", "Creates and then loads all level instances."),
	FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellLevelInstance"),
	EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::End));

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_OrganUnloadLevelInstances,
	"NWorldAssembly.NOrganComponent.UnloadProxies",
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_UnloadProxies", "Unload Level Instances"),
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_UnloadProxies_Tooltip", "Unload the level instances from the selected proxies"),
	FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellLevelInstance"),
	EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_OrganUnloadAllLevelInstances,
	"NWorldAssembly.NOrganComponent.UnloadAllProxies",
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_UnloadAllProxies", "Unload All Level Instances"),
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_UnloadAllProxies_Tooltip", "Unload all level instances."),
	FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NCellLevelInstance"),
	EUserInterfaceActionType::Button, FInputChord());

	// Build out quick assembly options
	CommandList_QuickAssembly = MakeShared<FUICommandList>();
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_QuickAssemblyToggleLoadInstances,
		"NWorldAssembly.QuickAssembly.ToggleLoadInstances",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_QuickAssembly_ToggleLoadInstances", "Load Cell Instances"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_QuickAssembly_ToggleLoadInstances_Tooltip", "Load the Cell Instances themselves from the generated ANCellProxy(s)."),
		FSlateIcon(), EUserInterfaceActionType::Check, FInputChord());
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_QuickAssemblyToggleAutoAssembly,
		"NWorldAssembly.QuickAssembly.ToggleAutoAssembly",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_QuickAssembly_ToggleAutoAssembly", "Enabled"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_QuickAssembly_ToggleAutoAssembly_Tooltip", "Should the selected organ continue to run assembly operations after completion, until stopped."),
		FSlateIcon(), EUserInterfaceActionType::Check, FInputChord());

	CommandList_QuickAssembly->MapAction(Get().CommandInfo_QuickAssemblyToggleLoadInstances,
		FExecuteAction::CreateStatic(&QuickAssemblyToggleLoadInstances),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&QuickAssemblyToggleLoadInstances_IsActionChecked));

	CommandList_QuickAssembly->MapAction(Get().CommandInfo_QuickAssemblyToggleAutoAssembly,
		FExecuteAction::CreateStatic(&QuickAssemblyToggleAutoAssembly),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&QuickAssemblyToggleAutoAssembly_IsActionChecked));

	// Map every Organ command from the single source of truth so this list and UnregisterGlobalActions
	// stay in lockstep — add or remove a command in GetGlobalOrganActions() and both sides pick it up.
	CommandList_Organ = MakeShared<FUICommandList>();
	for (const FNCommandInfoAction& Action : GetGlobalOrganActions())
	{
		CommandList_Organ->MapAction(Action.CommandInfo, Action.Execute, Action.CanExecute);
	}
}

TArray<FNWorldAssemblyEditorCommands::FNCommandInfoAction> FNWorldAssemblyEditorCommands::GetGlobalOrganActions() const
{
	return {
		{ CommandInfo_OrganGenerateProxies,         FExecuteAction::CreateStatic(&OrganGenerateProxies),      FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::CanGenerateSelectedOrgan) },
		{ CommandInfo_OrganGenerateAllProxies,      FExecuteAction::CreateStatic(&OrganGenerateAllProxies),   FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::CanGenerateAllOrgans) },
		{ CommandInfo_OrganClearProxies,            FExecuteAction::CreateStatic(&OrganClearGenerated),       FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::HasSelectedGeneratedCellProxies) },
		{ CommandInfo_OrganClearAllProxies,         FExecuteAction::CreateStatic(&OrganClearAllProxies),      FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::HasGeneratedCellProxies) },
		{ CommandInfo_OrganCreateLevelInstances,    FExecuteAction::CreateStatic(&OrganLoadProxyLevels),      FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::HasSelectedGeneratedCellProxies) },
		{ CommandInfo_OrganCreateAllLevelInstances, FExecuteAction::CreateStatic(&OrganLoadAllProxyLevels),   FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::HasGeneratedCellProxies) },
		{ CommandInfo_OrganUnloadLevelInstances,    FExecuteAction::CreateStatic(&OrganUnloadProxyLevels),    FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::HasSelectedGeneratedCellProxies) },
		{ CommandInfo_OrganUnloadAllLevelInstances, FExecuteAction::CreateStatic(&OrganUnloadAllProxyLevels), FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::HasGeneratedCellProxies) },
	};
}

void FNWorldAssemblyEditorCommands::RegisterGlobalActions(const TSharedRef<FUICommandList>& GlobalActions) const
{
	GlobalActions->Append(CommandList_Organ.ToSharedRef());
}

void FNWorldAssemblyEditorCommands::UnregisterGlobalActions(const TSharedRef<FUICommandList>& GlobalActions) const
{
	// Append (in RegisterGlobalActions) copies CommandList_Organ's bindings into GlobalActions, so each one has
	// to be unmapped by hand to detach the module statics it binds to. Both the mapping and this unmapping iterate
	// GetGlobalOrganActions(), so adding or removing an Organ command updates both sides, and they can't drift apart.
	for (const FNCommandInfoAction& Action : GetGlobalOrganActions())
	{
		GlobalActions->UnmapAction(Action.CommandInfo);
	}
}

void FNWorldAssemblyEditorCommands::WorldAssemblyEdMode()
{
	GLevelEditorModeTools().ActivateMode(FNWorldAssemblyEdMode::Identifier);
}

bool FNWorldAssemblyEditorCommands::WorldAssemblyEdMode_CanShow()
{
	if (FNWorldAssemblyEdMode::IsActive()) return false;
	return FNWorldAssemblyEditorUtils::IsOrganComponentPresentInCurrentWorld() || FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld();
}

void FNWorldAssemblyEditorCommands::CellActorEditHullMode()
{
	FNWorldAssemblyEdMode::SetCellEdMode(FNWorldAssemblyEdMode::ENCellEdMode::Hull);
}

bool FNWorldAssemblyEditorCommands::CellActorEditHullMode_CanExecute()
{
	const ULevel* Level = FNEditorUtils::GetCurrentLevel();
	if (Level == nullptr) return false;

	const UNCellRootComponent* RootComponent = FNWorldAssemblyRegistry::GetCellRootComponentFromLevel(Level);
	if (RootComponent == nullptr) return false;
	return !RootComponent->Details.Hull.HasNonTris();
}

void FNWorldAssemblyEditorCommands::CellActorEditBoundsMode()
{
	FNWorldAssemblyEdMode::SetCellEdMode(FNWorldAssemblyEdMode::ENCellEdMode::Bounds);
}

void FNWorldAssemblyEditorCommands::CellActorEditVoxelMode()
{
	FNWorldAssemblyEdMode::SetCellEdMode(FNWorldAssemblyEdMode::ENCellEdMode::Voxel);
}

void FNWorldAssemblyEditorCommands::CellActorToggleDrawVoxelData()
{
	switch (FNWorldAssemblyEdMode::GetCellVoxelMode())
	{
		using enum FNWorldAssemblyEdMode::ENCellVoxelMode;
	case Grid:
		FNWorldAssemblyEdMode::SetCellVoxelMode(Points);
		break;
	case None:
		FNWorldAssemblyEdMode::SetCellVoxelMode(Grid);
		break;
	default:
		FNWorldAssemblyEdMode::SetCellVoxelMode(None);
	}
}

void FNWorldAssemblyEditorCommands::CellAddActor()
{
	UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld == nullptr)
	{
		return;
	}

	if (FNEditorUtils::IsUnsavedWorld(CurrentWorld))
	{
		const EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Type::YesNo,
			FText::FromString(TEXT("You need to save the world/map that you are working in before creating a NCellActor.\n\nDo you wish to save the map now?")),
			FText::FromString(TEXT("NEXUS: World Assembly")));
		switch (Choice)
		{
		case EAppReturnType::No:
			UE_LOG(LogNexusWorldAssemblyEditor, Error, TEXT("Unable to add UNCellActor to an unsaved world."));
			return;
		case EAppReturnType::Yes:
			if (!FEditorFileUtils::SaveLevel(CurrentWorld->GetCurrentLevel()))
			{
				UE_LOG(LogNexusWorldAssemblyEditor, Error, TEXT("Unable to add UNCellActor to an unsaved world."));
				return;
			}
			break;
		default:
			UE_LOG(LogNexusWorldAssemblyEditor, Error, TEXT("Unable to add UNCellActor to an unsaved world."));
			return;
		}
	}

	ANCellActor* SpawnedActor = CurrentWorld->SpawnActor<ANCellActor>(ANCellActor::StaticClass(), FTransform::Identity, FActorSpawnParameters());
	if (SpawnedActor == nullptr)
	{
		UE_LOG(LogNexusWorldAssemblyEditor, Warning, TEXT("Failed to spawn the UNCellActor in the current world."));
		return;
	}

	// Apply default settings to the Cell
	const UNWorldAssemblyEditorSettings* Settings = UNWorldAssemblyEditorSettings::Get();
	Settings->ApplyDefaultSettings(SpawnedActor->CellRoot);

	FNWorldAssemblyEditorUtils::SaveCell(CurrentWorld, SpawnedActor);
}

bool FNWorldAssemblyEditorCommands::CellAddActor_CanShow()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;
	return FNWorldAssemblyEdMode::IsActive() && !FNWorldAssemblyEdMode::HasCellActor() && !FNWorldAssemblyEditorUtils::IsOrganComponentPresentInCurrentWorld();
}

void FNWorldAssemblyEditorCommands::OrganGenerateProxies()
{
	FNAssemblyOperationSettings EditorSettings = FNAssemblyOperationSettings::GetDefaultEditorSettings();
	UNWorldAssemblyEditorSubsystem::Get()->StartOperation(
		UNAssemblyOperation::CreateInstance(FNWorldAssemblyEditorUtils::GetSelectedOrganComponents(), EditorSettings));
}

void FNWorldAssemblyEditorCommands::OrganGenerateAllProxies()
{
	FNAssemblyOperationSettings EditorSettings = FNAssemblyOperationSettings::GetDefaultEditorSettings();
	UNWorldAssemblyEditorSubsystem::Get()->StartOperation(
		UNAssemblyOperation::CreateInstance(FNWorldAssemblyRegistry::GetOrganComponents(), EditorSettings));
}

void FNWorldAssemblyEditorCommands::OrganClearGenerated()
{
	TArray<UNOrganComponent*> SelectedOrganComponents = FNWorldAssemblyEditorUtils::GetSelectedOrganComponents();
	UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	for (UNOrganComponent* Component : SelectedOrganComponents)
	{
		Subsystem->ClearGenerated(Component->GetAndResetLastOperationTicket());
	}
}

void FNWorldAssemblyEditorCommands::OrganClearAllProxies()
{
	UNWorldAssemblyEditorSubsystem::Get()->ClearAllProxies();
}

void FNWorldAssemblyEditorCommands::OrganLoadProxyLevels()
{
	TArray<UNOrganComponent*> SelectedOrganComponents = FNWorldAssemblyEditorUtils::GetSelectedOrganComponents();
	UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	for (const UNOrganComponent* Component : SelectedOrganComponents)
	{
		Subsystem->LoadGeneratedProxies(Component->GetLastOperationTicket());
	}
}

void FNWorldAssemblyEditorCommands::OrganLoadAllProxyLevels()
{
	UNWorldAssemblyEditorSubsystem::Get()->LoadAllGeneratedProxies();
}

void FNWorldAssemblyEditorCommands::OrganUnloadProxyLevels()
{
	TArray<UNOrganComponent*> SelectedOrganComponents = FNWorldAssemblyEditorUtils::GetSelectedOrganComponents();
	UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	for (const UNOrganComponent* Component : SelectedOrganComponents)
	{
		Subsystem->UnloadGeneratedProxies(Component->GetLastOperationTicket());
	}
}

void FNWorldAssemblyEditorCommands::OrganUnloadAllProxyLevels()
{
	UNWorldAssemblyEditorSubsystem::Get()->UnloadAllGeneratedProxies();
}

void FNWorldAssemblyEditorCommands::CellSelectActor()
{
	GEditor->SelectNone(false, true);
	GEditor->SelectActor(FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld(),
		true, true, true, true);
}

bool FNWorldAssemblyEditorCommands::CellSelectActor_CanExecute()
{
	return FNWorldAssemblyEdMode::HasCellActor() && GEditor->CanSelectActor(FNWorldAssemblyEdMode::GetCellActor(), false);
}

bool FNWorldAssemblyEditorCommands::CellSelectActor_CanShow()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	// Use cached version
	return FNWorldAssemblyEdMode::IsActive() && FNWorldAssemblyEdMode::HasCellActor() &&  !FNWorldAssemblyEditorUtils::IsCellActorSelected();;
}

void FNWorldAssemblyEditorCommands::CellCalculateAll()
{
	CellCalculateBounds();
	CellCalculateHull();
	CellCalculateVoxelData();
}

void FNWorldAssemblyEditorCommands::CellCalculateBounds()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCommands_CellCalculateBounds", "Calculate Cell Bounds"));
	ANCellActor* CellActor = FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld();
	if (!ensure(CellActor != nullptr)) return;
	CellActor->CalculateBounds();
}

void FNWorldAssemblyEditorCommands::CellCalculateHull()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCommands_CellCalculateHull", "Calculate Cell Hull"));
	ANCellActor* CellActor = FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld();
	if (!ensure(CellActor != nullptr)) return;
	CellActor->CalculateHull();

	FNWorldAssemblyEdMode::ProtectCellEdMode();
}

void FNWorldAssemblyEditorCommands::CellCalculateVoxelData()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCommands_CellCalculateVoxel", "Calculate Voxel Data"));
	ANCellActor* CellActor = FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld();
	if (!ensure(CellActor != nullptr)) return;
	CellActor->CalculateVoxelData();
}

bool FNWorldAssemblyEditorCommands::CellCalculateVoxelData_CanExecute()
{
	const ANCellActor* CellActor = FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld();
	if (CellActor == nullptr) return false;
	return CellActor->GetCellRoot()->Details.VoxelSettings.bUseVoxelData;
}

void FNWorldAssemblyEditorCommands::CellToggleBoundsCalculateOnSave()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCommands_CellToggleBoundsCalculateOnSave", "Toggle Calculate Bounds On Save"));
	const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld != nullptr)
	{
		ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
		if (Actor != nullptr)
		{
			Actor->Modify();
			Actor->GetCellRoot()->Details.BoundsSettings.bCalculateOnSave = !Actor->GetCellRoot()->Details.BoundsSettings.bCalculateOnSave;
			Actor->SetActorDirty();
		}
	}
}

bool FNWorldAssemblyEditorCommands::CellToggleBoundsCalculateOnSave_IsActionChecked()
{
	const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld != nullptr)
	{
		const ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
		if ( Actor != nullptr)
		{
			return Actor->GetCellRoot()->Details.BoundsSettings.bCalculateOnSave;
		}
	}
	return false;
}

void FNWorldAssemblyEditorCommands::CellToggleHullCalculateOnSave()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCommands_CellToggleHullCalculateOnSave", "Toggle Calculate Hull On Save"));
	const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld != nullptr)
	{
		ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
		if (Actor != nullptr)
		{
			Actor->Modify();
			Actor->GetCellRoot()->Details.HullSettings.bCalculateOnSave = !Actor->GetCellRoot()->Details.HullSettings.bCalculateOnSave;
			Actor->SetActorDirty();
		}
	}
}

bool FNWorldAssemblyEditorCommands::CellToggleHullAllowNonConvex_IsActionChecked()
{
	const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld != nullptr)
	{
		const ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
		if ( Actor != nullptr)
		{
			return Actor->GetCellRoot()->Details.HullSettings.bAllowNonConvex;
		}
	}
	return false;
}

void FNWorldAssemblyEditorCommands::CellToggleHullAllowNonConvex()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCommands_CellToggleHullAllowConvex", "Toggle Allow Non-Convex Hull"));
	const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld != nullptr)
	{
		ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
		if (Actor != nullptr)
		{
			Actor->Modify();
			Actor->GetCellRoot()->Details.HullSettings.bAllowNonConvex = !Actor->GetCellRoot()->Details.HullSettings.bAllowNonConvex;
			if (Actor->GetCellRoot()->Details.HullSettings.bAllowNonConvex)
			{
				Actor->GetCellRoot()->Details.HullSettings.bCalculateOnSave = false;
			}
			Actor->SetActorDirty();
		}
	}
}

bool FNWorldAssemblyEditorCommands::CellToggleHullCalculateOnSave_IsActionChecked()
{
	const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld != nullptr)
	{
		const ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
		if (Actor != nullptr)
		{
			return Actor->GetCellRoot()->Details.HullSettings.bCalculateOnSave;
		}
	}
	return false;
}

void FNWorldAssemblyEditorCommands::CellToggleVoxelCalculateOnSave()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCommands_CellToggleVoxelCalculateOnSave", "Toggle Calculate Voxel On Save"));
	const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld != nullptr)
	{
		ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
		if (Actor != nullptr)
		{
			Actor->Modify();
			Actor->GetCellRoot()->Details.VoxelSettings.bCalculateOnSave = !Actor->GetCellRoot()->Details.VoxelSettings.bCalculateOnSave;
			Actor->SetActorDirty();
		}
	}
}

bool FNWorldAssemblyEditorCommands::CellToggleVoxelCalculateOnSave_IsActionChecked()
{
	const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld != nullptr)
	{
		const ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
		if (Actor != nullptr)
		{
			return Actor->GetCellRoot()->Details.VoxelSettings.bCalculateOnSave;
		}
	}
	return false;
}

void FNWorldAssemblyEditorCommands::CellToggleVoxelData()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCommands_CellToggleUseVoxelData", "Use Voxel Data"));
	const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld != nullptr)
	{
		ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
		if (Actor != nullptr)
		{
			Actor->Modify();
			Actor->GetCellRoot()->Details.VoxelSettings.bUseVoxelData = !Actor->GetCellRoot()->Details.VoxelSettings.bUseVoxelData;

			// Clear it here if someone uses the menu option
			if (!Actor->GetCellRoot()->Details.VoxelSettings.bUseVoxelData)
			{
				Actor->GetCellRoot()->Details.VoxelData = FNCellVoxelData();
			}
			Actor->SetActorDirty();
		}
	}
}

bool FNWorldAssemblyEditorCommands::CellToggleVoxelData_IsActionChecked()
{
	const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld != nullptr)
	{
		const ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
		if (Actor != nullptr)
		{
			return Actor->GetCellRoot()->Details.VoxelSettings.bUseVoxelData;
		}
	}
	return false;
}

void FNWorldAssemblyEditorCommands::CellResetCell()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCommands_CellResetCell", "Reset Cell"));
	// Get the cell actor
	ANCellActor* CellActor = FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld();
	if (!ensure(CellActor != nullptr)) return;
	CellActor->Modify();

	// Apply default settings to the Cell
	const UNWorldAssemblyEditorSettings* Settings = UNWorldAssemblyEditorSettings::Get();
	Settings->ApplyDefaultSettings(CellActor->CellRoot);

	// Get the cell
	UNCell* Cell = UAssetDefinition_NCell::GetOrCreatePackage( FNEditorUtils::GetCurrentWorld());
	if (Cell == nullptr)
	{
		UE_LOG(LogNexusWorldAssemblyEditor, Warning, TEXT("Unable to get or create the UNCell side-car package when trying to reset the cell."));
		return;
	}
	Cell->Root = FNCellRootDetails();
	Cell->Junctions.Empty();

	// Update the data
	FNWorldAssemblyEditorUtils::UpdateCell(Cell, CellActor);

	// Flag the actor as dirty
	CellActor->SetActorDirty();
}

void FNWorldAssemblyEditorCommands::CellSaveCell()
{
	UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	FNWorldAssemblyEditorUtils::SaveCell(CurrentWorld, FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true), true);
}

void FNWorldAssemblyEditorCommands::CellRemoveActor()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCommands_CellRemoveActor", "Remove Cell Actor"));
	UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	ANCellActor* CellActor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
	if (!ensure(CellActor != nullptr)) return;
	CellActor->Destroy();
	// ReSharper disable once CppExpressionWithoutSideEffects
	CurrentWorld->MarkPackageDirty();

	// Need to destroy the sidecar.
	if (const FString SidecarPath = UAssetDefinition_NCell::GetCellPackagePath(CurrentWorld->GetOutermost()->GetName());
		FPackageName::DoesPackageExist(SidecarPath))
		{
			UEditorAssetLibrary::DeleteAsset(SidecarPath);
		}
}

void FNWorldAssemblyEditorCommands::CellJunctionAddComponent()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCommands_CellJunctionAddComponent", "Add Junction Component"));
	TArray<UNCellJunctionComponent*> OutComponents;
	TArray<UNCellJunctionComponent*> SelectComponents;
	bool bNeedsRefresh = false;
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		AActor* Actor = Cast<AActor>( *SelectedActor );
		if (!Actor) continue;

		auto NewComponent = static_cast<UNCellJunctionComponent*>(Actor->AddComponentByClass(
			UNCellJunctionComponent::StaticClass(), true, FTransform::Identity, false));

		Actor->Modify();
		NewComponent->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
		Actor->AddInstanceComponent(NewComponent);
		NewComponent->SetFlags(RF_Transactional);
		NewComponent->RegisterComponent();

		SelectComponents.Add(NewComponent);
		bNeedsRefresh = true;

	}

	// Refresh the details panel if needed
	if (bNeedsRefresh)
	{
		GEditor->SelectNone(false, true);
		for (UNCellJunctionComponent* Component : SelectComponents)
		{
			GEditor->SelectComponent(Component, true, true, true);
		}
	}
}

void FNWorldAssemblyEditorCommands::CellJunctionSelectComponent(UNCellJunctionComponent* Junction)
{
	GEditor->SelectNone(false, true);
	GEditor->SelectComponent(Junction, true, true, true);
}

void FNWorldAssemblyEditorCommands::CellJunctionCollectComponents()
{
	UEditorActorSubsystem* EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	if (EditorActorSubsystem)
	{
		TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
		if (SelectedActors.Num() > 0)
		{
			AActor* TargetActor = SelectedActors[0];

			// A location-locked actor (such as the ANCellActor, which pins itself to the origin) propagates its lock to
			// every scene component it owns: USceneComponent::CanEditChange disables RelativeLocation/RelativeRotation/
			// RelativeScale3D on components owned by a location-locked actor. Collecting junctions onto such an actor would
			// leave them unmovable in the editor (Details transform greyed out, scale gizmo disabled), so bail out with a
			// warning rather than silently breaking junction authoring.
			if (TargetActor->IsLockLocation())
			{
				const TCHAR* TargetTypeName = TargetActor->IsA<ANCellActor>()
					? TEXT("NEXUS Cell Actor")
					: TEXT("location-locked actor");

				UE_LOG(LogNexusWorldAssemblyEditor, Warning,
					TEXT("Aborting Collect Junction Components: target actor '%s' is a %s and is location-locked; junctions parented to it would become unmovable in the editor."),
					*TargetActor->GetActorLabel(), TargetTypeName);

				const FText WarningMessage = FText::FromString(FString::Printf(
					TEXT("'%s' is a %s and is locked to its location.\n\nCollecting junctions onto it would lock their transforms — Unreal disables moving, rotating, and scaling components owned by a location-locked actor, so the junctions could no longer be edited.\n\nSelect a non-locked actor as the collection target instead."),
					*TargetActor->GetActorLabel(), TargetTypeName));

				FMessageDialog::Open(EAppMsgCategory::Warning, EAppMsgType::Type::Ok,
					WarningMessage, FText::FromString(TEXT("NEXUS: World Assembly")));
				return;
			}

			// Snapshot the registry: reparenting unregisters/re-registers each junction, which mutates the live
			// registry array (via OnUnregister/OnRegister), so iterating the reference directly would skip entries.
			TArray<UNCellJunctionComponent*> CellJunctions = FNWorldAssemblyRegistry::GetCellJunctionComponents();
			if (CellJunctions.Num() == 0)
			{
				return;
			}

			// Confirm the move with the user, listing the junctions and their destination actor.
			constexpr int32 MaxListedJunctions = 25;
			FString JunctionList;
			for (int32 Index = 0; Index < CellJunctions.Num(); ++Index)
			{
				if (Index >= MaxListedJunctions)
				{
					JunctionList.Append(FString::Printf(TEXT("\n  - ...and %d more"), CellJunctions.Num() - MaxListedJunctions));
					break;
				}
				JunctionList.Append(FString::Printf(TEXT("\n  - %s"), *CellJunctions[Index]->GetJunctionName()));
			}

			const FText ConfirmMessage = FText::FromString(FString::Printf(
				TEXT("This will move %d junction(s) to '%s':\n%s\n\nDo you wish to proceed?"),
				CellJunctions.Num(), *TargetActor->GetActorLabel(), *JunctionList));

			const EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgCategory::Warning, EAppMsgType::Type::YesNo,
				ConfirmMessage, FText::FromString(TEXT("NEXUS: World Assembly")));
			if (Choice != EAppReturnType::Yes)
			{
				return;
			}

			const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCommands_CellJunctionCollectComponents", "Collect Junction Components"));
			TargetActor->Modify();
			const FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, true);
			for (UNCellJunctionComponent* Component : CellJunctions)
			{
				AActor* PreviousOwner = Component->GetOwner();

				Component->SetFlags(RF_Transactional);
				Component->Modify();
				if (PreviousOwner)
				{
					PreviousOwner->Modify();
				}

				// Tear down render/physics state before reparenting the component.
				Component->UnregisterComponent();
				Component->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

				// Drop the component from the previous owner's bookkeeping.
				if (PreviousOwner)
				{
					PreviousOwner->RemoveInstanceComponent(Component);
					PreviousOwner->RemoveOwnedComponent(Component);
				}

				// Reparent the UObject to the target actor and register with its bookkeeping + scene.
				Component->Rename(nullptr, TargetActor, REN_DontCreateRedirectors);
				TargetActor->AddOwnedComponent(Component);
				TargetActor->AddInstanceComponent(Component);
				Component->AttachToComponent(TargetActor->GetRootComponent(), AttachmentRules);
				Component->RegisterComponent();
			}

			// Flag the level so the change is picked up by Save.
			if (UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld())
			{
				// ReSharper disable once CppExpressionWithoutSideEffects
				CurrentWorld->MarkPackageDirty();
			}

			// Refresh the Details panel / components tree for the current selection without re-selecting the actor.
			FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
			LevelEditorModule.BroadcastComponentsEdited();
		}
	}
}

void FNWorldAssemblyEditorCommands::OrganSelectComponent(UNOrganComponent* Organ)
{
	GEditor->SelectNone(false, true);
	GEditor->SelectComponent(Organ, true, true, true);
}

void FNWorldAssemblyEditorCommands::CellCaptureThumbnail()
{
	FViewport* Viewport = GEditor->GetActiveViewport();
	if (Viewport != nullptr &&
		ensure(GCurrentLevelEditingViewportClient) && ensure(Viewport) )
	{
		FLevelEditorViewportClient* OldViewportClient = GCurrentLevelEditingViewportClient;
		GCurrentLevelEditingViewportClient = nullptr;
		TArray<FAssetData> SelectedAssets;
		const FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		// Level
		const UWorld* World = FNEditorUtils::GetCurrentWorld();
		if (World != nullptr)
		{
			FAssetData LevelAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(World));
			SelectedAssets.Emplace(LevelAssetData);
		}
		if (SelectedAssets.Num() > 0)
		{
			FNWorldAssemblyEdMode::SetRenderMode(ENWorldAssemblyEdModeRenderMode::LevelScreenshot);
			Viewport->Draw();
			AssetViewUtils::CaptureThumbnailFromViewport(Viewport, SelectedAssets);
		}

		// Cell Data
		SelectedAssets.Empty();
		ANCellActor* CellActor = FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld();
		if (CellActor != nullptr)
		{
			FAssetData CellAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(CellActor->Sidecar.ToSoftObjectPath()));
			SelectedAssets.Emplace(CellAssetData);
		}
		if (SelectedAssets.Num() > 0)
		{
			FNWorldAssemblyEdMode::SetRenderMode(ENWorldAssemblyEdModeRenderMode::CellScreenshot);
			Viewport->Draw();
			AssetViewUtils::CaptureThumbnailFromViewport(Viewport, SelectedAssets);
		}

		GCurrentLevelEditingViewportClient = OldViewportClient;
		FNWorldAssemblyEdMode::SetRenderMode(ENWorldAssemblyEdModeRenderMode::All);
		Viewport->Draw();
	}
}

bool FNWorldAssemblyEditorCommands::CellCaptureThumbnail_CanExecute()
{
	const UWorld* World = FNEditorUtils::GetCurrentWorld();
	return World != nullptr && !FNEditorUtils::IsUnsavedWorld(World);
}

void FNWorldAssemblyEditorCommands::QuickAssemblyButtonClicked()
{
	// "Active" spans both a running operation and the wait between auto-assembly runs, so a click during the
	// inter-run gap cancels the loop rather than kicking off a second, overlapping operation.
	if (FNWorldAssemblyEditorToolMenu::IsQuickAssemblyActive())
	{
		CancelQuickAssembly();
	}
	else
	{
		StartQuickAssembly();
	}
}

bool FNWorldAssemblyEditorCommands::QuickAssemblyButton_CanExecute()
{
	// While the loop is active (operation running or waiting between auto-runs) the button must stay enabled so the
	// user can cancel it; otherwise fall back to the start preconditions (which require that no operation is running).
	if (FNWorldAssemblyEditorToolMenu::IsQuickAssemblyActive()) return true;
	return StartQuickAssembly_CanExecute();
}

void FNWorldAssemblyEditorCommands::CancelQuickAssembly()
{
	// Disengage the auto-assembly loop first so the pending operation's teardown can't re-arm the inter-run timer.
	// When cancelled during the inter-run gap (no live operation) this also clears the toolbar countdown bar. A
	// cancel is a deliberate user stop, so surface the accumulated pass/warn/fail summary for any completed runs.
	UNWorldAssemblyEditorSubsystem::Get()->StopAutoAssemblyLoop(/*bEmitSummary*/ true);

	if (UNAssemblyOperation* Operation = FNWorldAssemblyEditorToolMenu::GetTrackedQuickAssemblyOperation();
		Operation != nullptr && Operation->IsRunning())
	{
		// Synchronous: cancels the task graph and tears the operation down, which routes through the subsystem's
		// OnOperationDestroyed to clear the progress bar and reset the tracked ticket.
		Operation->Cancel();
	}
}

void FNWorldAssemblyEditorCommands::QuickAssemblyToggleLoadInstances()
{
	UNWorldAssemblyEditorUserSettings* Settings = UNWorldAssemblyEditorUserSettings::GetMutable();
	Settings->bQuickAssemblyLoadLevelInstances = !Settings->bQuickAssemblyLoadLevelInstances;
	Settings->SaveConfig();
}

bool FNWorldAssemblyEditorCommands::QuickAssemblyToggleLoadInstances_IsActionChecked()
{
	return UNWorldAssemblyEditorUserSettings::Get()->bQuickAssemblyLoadLevelInstances;
}

void FNWorldAssemblyEditorCommands::QuickAssemblyToggleAutoAssembly()
{
	UNWorldAssemblyEditorUserSettings* Settings = UNWorldAssemblyEditorUserSettings::GetMutable();
	Settings->bQuickAssemblyAutoAssembly = !Settings->bQuickAssemblyAutoAssembly;
	Settings->SaveConfig();
}

bool FNWorldAssemblyEditorCommands::QuickAssemblyToggleAutoAssembly_IsActionChecked()
{
	return UNWorldAssemblyEditorUserSettings::Get()->bQuickAssemblyAutoAssembly;
}

void FNWorldAssemblyEditorCommands::StartQuickAssembly()
{
	UNOrganComponent* Component = FNWorldAssemblyEditorToolMenu::GetQuickAssemblyOrganComponent();
	if (Component == nullptr) return;

	UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	Subsystem->ClearGenerated(Component->GetAndResetLastOperationTicket());

	const UNWorldAssemblyEditorUserSettings* UserSettings = UNWorldAssemblyEditorUserSettings::Get();

	FNAssemblyOperationSettings EditorSettings = FNAssemblyOperationSettings::GetDefaultEditorSettings();
	EditorSettings.bCreateLevelInstances = UserSettings->bQuickAssemblyLoadLevelInstances;

	UNAssemblyOperation* Operation = UNAssemblyOperation::CreateInstance(Component, EditorSettings);
	FNWorldAssemblyEditorToolMenu::SetQuickAssemblyOperationTicket(Operation->GetTicket());

	// Drive the toolbar progress bar from the operation's combined task + sub-channel progress, so it keeps
	// moving during long-running tasks. The delegate lives on the operation, so it auto-detaches when the
	// operation is destroyed - no manual unbind needed.
	FNWorldAssemblyEditorToolMenu::SetQuickAssemblyProgress(0.0f); // Show an empty bar immediately.
	Operation->OnPercentageChanged.AddDynamic(Subsystem, &UNWorldAssemblyEditorSubsystem::OnQuickAssemblyProgressChanged);

	Subsystem->StartOperation(Operation);

	// With Auto Assembly enabled, engage the loop so the button stays in its cancel state through the inter-run
	// waits; the subsystem re-arms the timer each time the operation finishes (this same function is re-entered
	// from the timer, so settings are re-read live every run). Guard on IsRunning so a no-op StartOperation
	// (e.g. invalid context) doesn't leave the loop engaged with nothing scheduled.
	if (Operation->IsRunning() && UserSettings->bQuickAssemblyAutoAssembly)
	{
		Subsystem->BeginAutoAssemblyLoop();
	}
}

bool FNWorldAssemblyEditorCommands::StartQuickAssembly_CanExecute()
{
	return	FNWorldAssemblyEditorToolMenu::HasValidQuickAssemblyOrgan() &&
			FNEditorUtils::IsNotPlayInEditor() &&
			!FNWorldAssemblyRegistry::HasOperations();
}

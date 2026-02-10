// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NFixersEditorCommands.h"

#include "NEditorCommands.h"
#include "Fixers/NPoseAssetFixer.h"

void FNFixersEditorCommands::RegisterCommands()
{
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_BulkOperations_PoseAsset_OutOfDateAnimationSource,
		"NFixers.BulkOperations.PoseAsset.OutOfDateAnimationSource",
		NSLOCTEXT("NexusFixersEditor", "Command_PoseAsset", "Outdated PoseAsset Source Animations"),
		NSLOCTEXT("NexusFixersEditor", "Command_BulkOperations", "Find and fix any PoseAssets in the selected content with out-of-date source animations."),
		FSlateIcon(FNFixersEditorStyle::GetStyleSetName(), "Command.FindAndFix.Item"),
		EUserInterfaceActionType::Button, FInputChord());

	CommandList_BulkOperations = MakeShared<FUICommandList>();
	
	CommandList_BulkOperations->MapAction(Get().CommandInfo_BulkOperations_PoseAsset_OutOfDateAnimationSource,
		FExecuteAction::CreateStatic(&FNPoseAssetFixer::OutOfDateAnimationSource, true));
	
	
	// Add Tool Commands
	FNEditorCommandInfo CommandInfo;
	CommandInfo.Identifier = "NEXUS_Tools_FindAndFix_PoseAsset_OutOfDateAnimationSource";
	CommandInfo.Section = "Find & Fix";
	CommandInfo.DisplayName = NSLOCTEXT("NexusFixersEditor", "Command_PoseAsset", "Outdated PoseAsset Source Animations");
	CommandInfo.Tooltip = NSLOCTEXT("NexusFixersEditor", "Command_BulkOperations", "Find and fix any PoseAssets in the selected content with out-of-date source animations.");
	CommandInfo.Icon = FSlateIcon(FNFixersEditorStyle::GetStyleSetName(), "Command.FindAndFix.Item");
	CommandInfo.bContextMenu = true;
	FNEditorCommands::AddToolCommand(CommandInfo);
}

void FNFixersEditorCommands::UnregisterCommands()
{
	FNEditorCommands::RemoveToolCommand("NEXUS_Tools_FindAndFix_PoseAsset_OutOfDateAnimationSource");
}

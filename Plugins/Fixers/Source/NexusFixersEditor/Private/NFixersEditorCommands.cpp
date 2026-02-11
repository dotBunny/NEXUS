// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NFixersEditorCommands.h"

#include "NEditorCommands.h"
#include "NFixersEditorBulkOperations.h"
#include "NFixersEditorStyle.h"
#include "Fixers/NPoseAssetFixer.h"

void FNFixersEditorCommands::Register()
{
	// Add Tool Commands
	FNEditorCommandInfo CommandInfo;
	CommandInfo.Identifier = "NEXUS_Tools_FindAndFix_PoseAsset_OutOfDateAnimationSource";
	CommandInfo.Section = "Assets";
	CommandInfo.DisplayName = NSLOCTEXT("NexusFixersEditor", "Command_PoseAsset", "Outdated PoseAsset Source Animations");
	CommandInfo.Tooltip = NSLOCTEXT("NexusFixersEditor", "Command_BulkOperations", "Find and fix any PoseAssets in the selected content with out-of-date source animations.");
	CommandInfo.Icon = FSlateIcon(FNFixersEditorStyle::GetStyleSetName(), "Command.FindAndFix.Item");
	CommandInfo.Execute = FExecuteAction::CreateStatic(&FNPoseAssetFixer::OutOfDateAnimationSource, true);
	FNFixersEditorBulkOperations::AddCommand(CommandInfo);
}

void FNFixersEditorCommands::Unregister()
{
	FNFixersEditorBulkOperations::RemoveCommand("NEXUS_Tools_FindAndFix_PoseAsset_OutOfDateAnimationSource");
}

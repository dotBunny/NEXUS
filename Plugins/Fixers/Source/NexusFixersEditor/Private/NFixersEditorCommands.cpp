// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NFixersEditorCommands.h"
#include "Fixers/NPoseAssetFixer.h"

void FNFixersEditorCommands::RegisterCommands()
{
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_BulkOperations_PoseAsset_OutOfDateAnimationSource,
		"NFixers.BulkOperations.PoseAsset.OutOfDateAnimationSource",
		NSLOCTEXT("NexusFixersEditor", "Command_PoseAsset", "Outdated PoseAsset Source Animations"),
		NSLOCTEXT("NexusFixersEditor", "Command_BulkOperations", "Find and fix any PoseAssets in the selected content with out-of-date source animations."),
		FSlateIcon(FNFixersEditorStyle::GetStyleSetName(), "Command.FindAndFix.Item"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_BulkOperations_PoseAsset_OutOfDateAnimationSource_NoContext,
		"NFixers.BulkOperations.PoseAsset.OutOfDateAnimationSource.NoContext",
		NSLOCTEXT("NexusFixersEditor", "Command_PoseAsset", "Outdated PoseAsset Source Animations"),
		NSLOCTEXT("NexusFixersEditor", "Command_BulkOperations_NoContext", "Find and fix any PoseAssets in the game content with out-of-date source animations."),
		FSlateIcon(FNFixersEditorStyle::GetStyleSetName(), "Command.FindAndFix.Item"),
		EUserInterfaceActionType::Button, FInputChord());

	CommandList_BulkOperations = MakeShared<FUICommandList>();
	
	CommandList_BulkOperations->MapAction(Get().CommandInfo_BulkOperations_PoseAsset_OutOfDateAnimationSource_NoContext,
		FExecuteAction::CreateStatic(&FNPoseAssetFixer::OutOfDateAnimationSource, false),
		FCanExecuteAction::CreateStatic(&FNPoseAssetFixer::OutOfDateAnimationSource_CanExecute));
	
	CommandList_BulkOperations->MapAction(Get().CommandInfo_BulkOperations_PoseAsset_OutOfDateAnimationSource,
		FExecuteAction::CreateStatic(&FNPoseAssetFixer::OutOfDateAnimationSource, true),
		FCanExecuteAction::CreateStatic(&FNPoseAssetFixer::OutOfDateAnimationSource_CanExecute));
}
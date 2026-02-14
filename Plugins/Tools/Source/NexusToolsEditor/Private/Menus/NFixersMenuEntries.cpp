// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Menus/NFixersMenuEntries.h"

#include "NToolsEditorStyle.h"
#include "Fixers/NPoseAssetFixer.h"
#include "Menus/NFixersMenu.h"
#include "Menus/NMenuEntry.h"

void FNFixersMenuEntries::AddMenuEntries()
{
	// Add Tool Commands
	FNMenuEntry FixPoseAssetMenuEntry = FNMenuEntry();
	FixPoseAssetMenuEntry.Identifier = "PoseAsset_OutOfDateAnimationSource";
	FixPoseAssetMenuEntry.Section = "Assets";
	FixPoseAssetMenuEntry.DisplayName = NSLOCTEXT("NexusToolsEditor", "PoseAsset_OutOfDateAnimationSource", "Outdated PoseAsset Source Animations");
	FixPoseAssetMenuEntry.Tooltip = NSLOCTEXT("NexusToolsEditor", "PoseAsset_OutOfDateAnimationSource_Tooltip", "Find and fix any PoseAssets in the selected content with out-of-date source animations.");
	FixPoseAssetMenuEntry.Icon = FSlateIcon(FNToolsEditorStyle::GetStyleSetName(), "Command.FindAndFix.Item");
	FixPoseAssetMenuEntry.Execute = FExecuteAction::CreateStatic(&FNPoseAssetFixer::OutOfDateAnimationSource, true);
	FNFixersMenu::AddMenuEntry(FixPoseAssetMenuEntry);
}

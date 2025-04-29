// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NFixersEditorBulkOperations.h"
#include "NFixersEditorCommands.h"

#define LOCTEXT_NAMESPACE "NexusFixersEditor"

void FNFixersEditorBulkOperations::Register()
{
	// Add to folder to ContentBrowser
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.FolderContextMenu"))
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("PathContextBulkOperations");
		Section.AddSubMenu(
				"NFixersBulkOperations",
				LOCTEXT("NFixersBulkOperations", "Find & Fix"),
				LOCTEXT("NFixersBulkOperations_ToolTip", "Find and fix operations on selected content."),
				FNewToolMenuDelegate::CreateStatic(&FillMenu, true),
				false,
				FSlateIcon(FNFixersEditorStyle::GetStyleSetName(), "Command.FindAndFix")
			);
	}

	// Add to tools menu
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools"))
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("NEXUS");
		Section.Label = LOCTEXT("NLevelEditorTools", "NEXUS");

		Section.AddSubMenu(
				"NFixersBulkOperations",
				LOCTEXT("NFixersBulkOperations", "Find & Fix"),
				LOCTEXT("NFixersBulkOperations_ToolTip", "Find and fix operations on game content."),
				FNewToolMenuDelegate::CreateStatic(&FillMenu, false),
				false,
				FSlateIcon(FNFixersEditorStyle::GetStyleSetName(), "Command.FindAndFix")
			);
	}
}

void FNFixersEditorBulkOperations::Unregister()
{
}

void FNFixersEditorBulkOperations::FillMenu(UToolMenu* Menu, bool bIsContextMenu)
{
	const FNFixersEditorCommands Commands = FNFixersEditorCommands::Get();
	FToolMenuSection& AssetsSection = Menu->AddSection(TEXT("Assets"));
	AssetsSection.Label = LOCTEXT("NFixersBulkOperations_Assets", "Assets");
	
	if (bIsContextMenu)
	{
		AssetsSection.AddMenuEntryWithCommandList(Commands.CommandInfo_BulkOperations_PoseAsset_OutOfDateAnimationSource,
			Commands.CommandList_BulkOperations);
	}
	else
	{
		AssetsSection.AddMenuEntryWithCommandList(Commands.CommandInfo_BulkOperations_PoseAsset_OutOfDateAnimationSource_NoContext,
			Commands.CommandList_BulkOperations);
	}
}

#undef LOCTEXT_NAMESPACE
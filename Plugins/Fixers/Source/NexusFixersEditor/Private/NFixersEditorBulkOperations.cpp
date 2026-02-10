// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NFixersEditorBulkOperations.h"

#include "NEditorCommands.h"
#include "NFixersEditorCommands.h"

void FNFixersEditorBulkOperations::Register()
{
	// Add to folder to ContentBrowser
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.FolderContextMenu"))
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("PathContextBulkOperations");
		Section.AddSubMenu(
				"NFixersBulkOperations",
				NSLOCTEXT("NexusFixersEditor", "ContextMenu_FindAndFix", "Find & Fix"),
				NSLOCTEXT("NexusFixersEditor", "ContextMenu_FindAndFix_ToolTip", "Find and fix operations on selected content."),
				FNewToolMenuDelegate::CreateStatic(&FillMenu, true),
				false,
				FSlateIcon(FNFixersEditorStyle::GetStyleSetName(), "Command.FindAndFix")
			);
	}


}

void FNFixersEditorBulkOperations::Unregister()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (ToolMenus != nullptr)
	{
		ToolMenus->RemoveEntry("ContentBrowser.FolderContextMenu", "PathContextBulkOperations", "NFixersBulkOperations");
	}
}

void FNFixersEditorBulkOperations::FillMenu(UToolMenu* Menu, bool bIsContextMenu)
{
	const FNFixersEditorCommands Commands = FNFixersEditorCommands::Get();
	FToolMenuSection& AssetsSection = Menu->AddSection(TEXT("Assets"));
	AssetsSection.Label = NSLOCTEXT("NexusFixersEditor", "NFixersBulkOperations_Assets", "Assets");
	if (bIsContextMenu)
		AssetsSection.AddMenuEntryWithCommandList(Commands.CommandInfo_BulkOperations_PoseAsset_OutOfDateAnimationSource,
					Commands.CommandList_BulkOperations);
}
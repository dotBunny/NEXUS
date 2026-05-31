// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorStyle.h"

/**
 * Editor command bindings used by the NexusCoreEditor to inject help, bug-report, and documentation menu items
 * into the main editor menu bar.
 */
class FNEditorCommands final : public TCommands<FNEditorCommands>
{
public:
	FNEditorCommands()
		: TCommands<FNEditorCommands>(
			TEXT("NEditorCommands"),
			NSLOCTEXT("Contexts", "NEditorCommands", "NEditorCommands"),
			NAME_None,
			FNEditorStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

	/** Inserts NEXUS help entries into the main editor menu. */
	static void AddMenuEntries();

	/** Removes NEXUS help entries from the main editor menu. */
	static void RemoveMenuEntries();

	/** Handler: opens the Overwatch crash-reporter dashboard. */
	static void OnHelpOverwatch();
	/** Handler: opens the public NEXUS Issues list. */
	static void OnHelpIssues();
	/** Handler: opens the NEXUS bug-report form. */
	static void OnHelpBugReport();
	/** Handler: opens the NEXUS Discord. */
	static void OnHelpDiscord();
	/** Handler: opens the NEXUS public roadmap. */
	static void OnHelpRoadmap();
	/** Handler: opens the NEXUS documentation site. */
	static void OnHelpDocumentation();

	/**
	 * Builds the NEXUS help submenu shown inside the editor's Help menu.
	 * @param Menu Tool menu being populated.
	 */
	static void GenerateHelpSubMenu(UToolMenu* Menu);

private:
	TSharedPtr<FUICommandList> CommandList_Help;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_Discord;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_BugReport;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_Overwatch;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_Roadmap;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_Issues;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_Documentation; 

	TSharedPtr<FUICommandList> CommandList_Node;
	TSharedPtr<FUICommandInfo> CommandInfo_Node_ExternalDocumentation;
};
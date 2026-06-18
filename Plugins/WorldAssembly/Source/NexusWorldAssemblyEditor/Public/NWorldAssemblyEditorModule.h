// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWorldAssemblyEditorStyle.h"
#include "Macros/NModuleMacros.h"
#include "Menus/NToolsMenuMacros.h"
#include "Modules/ModuleInterface.h"

class FNCellRootComponentVisualizer;
class FPlacementModeID;
class FNWorldAssemblyEditorUndo;

/**
 * NEXUS: Procedural Generation Editor Module
 */
class FNWorldAssemblyEditorModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	N_MODULE_BASE(FNWorldAssemblyEditorModule, "NexusWorldAssemblyEditor")

	/** The cell-root component visualizer registered by this module on startup. */
	TSharedPtr<FNCellRootComponentVisualizer> RootComponentVisualizer;
	
private:
	/** Custom undo handler that intercepts World Assembly-related transactions. */
	TSharedPtr<FNWorldAssemblyEditorUndo> UndoHandler;

	/** Handles of placement-mode entries this module registered (used to unregister on shutdown). */
	TArray<TOptional<FPlacementModeID>> PlacementActors;

	/** Engine-init callback: completes registration once the editor is fully up. */
	void OnPostEngineInit();
};

N_TOOLS_MENU_EUW_ENTRY(
	"Developer Overlay", EUW_NWorldAssemblySystem,
	NSLOCTEXT("NexusWorldAssemblyEditor", "Create_EUW_DisplayName", "World Assembly"), 
	NSLOCTEXT("NexusWorldAssemblyEditor", "Create_EUW_Tooltip", "Opens the NWorldAssembly Developer Overlay inside of an editor tab."),
	FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Icon.WorldAssembly"),
	"/NexusWorldAssembly/EditorResources/EUW_NWorldAssembly.EUW_NWorldAssembly")
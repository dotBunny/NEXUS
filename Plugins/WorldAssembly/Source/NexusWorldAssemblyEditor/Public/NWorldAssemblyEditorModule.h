// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWorldAssemblyEditorStyle.h"
#include "Macros/NModuleMacros.h"
#include "Menus/NToolsMenuMacros.h"
#include "Modules/ModuleInterface.h"

class FComponentVisualizer;
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
	/**
	 * @note Held only so ShutdownModule can release it. Nothing calls back into the derived type any more — the
	 *       selection state the tool menu used to reach for lives on UNCellHullTool now — so the base interface is
	 *       all this needs to be.
	 */
	TSharedPtr<FComponentVisualizer> RootComponentVisualizer;

private:
	/** Custom undo handler that intercepts World Assembly-related transactions. */
	TSharedPtr<FNWorldAssemblyEditorUndo> UndoHandler;

	/** Handles of placement-mode entries this module registered (used to unregister on shutdown). */
	TArray<TOptional<FPlacementModeID>> PlacementActors;

	/** Handle for the asset-registry rename hook that moves a cell side-car alongside its world; removed on shutdown. */
	FDelegateHandle AssetRenamedHandle;

	/** Handle for the asset-registry remove hook that cleans up an orphaned cell side-car; removed on shutdown. */
	FDelegateHandle AssetRemovedHandle;

	/** Handle for the world pre-save hook that syncs cell data into its side-car; removed on shutdown. */
	FDelegateHandle PreSaveWorldHandle;

	/** Handle for the world post-save hook that flushes the dirtied side-car to disk; removed on shutdown. */
	FDelegateHandle PostSaveWorldHandle;

	/** Engine-init callback: completes registration once the editor is fully up. */
	void OnPostEngineInit();
};

N_TOOLS_MENU_EUW_ENTRY(
	"Developer Overlay", EUW_NWorldAssemblySystem,
	NSLOCTEXT("NexusWorldAssemblyEditor", "Create_EUW_DisplayName", "World Assembly"),
	NSLOCTEXT("NexusWorldAssemblyEditor", "Create_EUW_Tooltip", "Opens the NWorldAssembly Developer Overlay inside of an editor tab."),
	FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Icon.WorldAssembly"),
	"/NexusWorldAssembly/EditorResources/EUW_NWorldAssembly.EUW_NWorldAssembly")
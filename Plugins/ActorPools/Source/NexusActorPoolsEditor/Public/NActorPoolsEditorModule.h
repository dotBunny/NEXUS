// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NActorPoolsEditorStyle.h"
#include "Macros/NModuleMacros.h"
#include "Menus/NToolsMenuMacros.h"
#include "Modules/ModuleInterface.h"

class FPlacementModeID;
/**
 * NEXUS: Actor Pools Editor Module
 */
class FNActorPoolsEditorModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	N_IMPLEMENT_MODULE(FNActorPoolsEditorModule, "NexusActorPoolsEditor")

private:
	/** Registered placement-mode entries, tracked so they can be unregistered on shutdown. */
	TArray<TOptional<FPlacementModeID>> PlacementActors;
	void OnPostEngineInit();
};

N_IMPLEMENT_TOOLS_MENU_EUW_ENTRY(
	"Developer Overlay", EUW_NActorPools,
	NSLOCTEXT("NexusActorPoolsEditor", "Create_EUW_DisplayName", "Actor Pools"), 
	NSLOCTEXT("NexusActorPoolsEditor", "Create_EUW_Tooltip", "Opens the NActorPools Developer Overlay inside of an editor tab."),
	FSlateIcon(FNActorPoolsEditorStyle::GetStyleSetName(), "ClassIcon.NActorPool"),
	"/NexusActorPools/EditorResources/EUW_NActorPools.EUW_NActorPools")
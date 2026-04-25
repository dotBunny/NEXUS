// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDynamicRefsEditorStyle.h"
#include "Macros/NModuleMacros.h"
#include "Menus/NToolsMenuMacros.h"
#include "Modules/ModuleInterface.h"

/**
 * NEXUS: DynamicRefs Editor Module
 */
class FNDynamicRefsEditorModule final : public IModuleInterface
{
public:	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	N_IMPLEMENT_MODULE(FNDynamicRefsEditorModule, "NexusDynamicRefsEditor")
	
private:	
	void OnPostEngineInit();
};

N_IMPLEMENT_TOOLS_MENU_EUW_ENTRY(
	"Developer Overlay", EUW_NDynamicRefs,
	NSLOCTEXT("NexusDynamicRefsEditor", "Create_EUW_DisplayName", "Dynamic References"), 
	NSLOCTEXT("NexusDynamicRefsEditor", "Create_EUW_Tooltip", "Opens the NDynamicRefs Developer Overlay inside of an editor tab."),
	FSlateIcon(FNDynamicRefsEditorStyle::GetStyleSetName(), "ClassIcon.NDynamicRefComponent"),
	"/NexusDynamicRefs/EditorResources/EUW_NDynamicRefs.EUW_NDynamicRefs")

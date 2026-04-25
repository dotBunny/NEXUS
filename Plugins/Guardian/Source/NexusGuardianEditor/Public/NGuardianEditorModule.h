// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NGuardianEditorStyle.h"
#include "Macros/NModuleMacros.h"
#include "Menus/NToolsMenuMacros.h"
#include "Modules/ModuleInterface.h"

/**
 * NEXUS: Guardian Editor Module
 */
class FNGuardianEditorModule final : public IModuleInterface
{
public:	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	N_IMPLEMENT_MODULE(FNGuardianEditorModule, "NexusGuardianEditor")
	
private:	
	void OnPostEngineInit();
};

N_IMPLEMENT_TOOLS_MENU_EUW_ENTRY(
	"Developer Overlay", EUW_NGuardian,
	NSLOCTEXT("NexusGuardianEditor", "Create_EUW_DisplayName", "Guardian"), 
	NSLOCTEXT("NexusGuardianEditor", "Create_EUW_Tooltip", "Opens the NGuardian Developer Overlay inside of an editor tab."),
	FSlateIcon(FNGuardianEditorStyle::GetStyleSetName(), "Icon.NGuardian"),
	"/NexusGuardian/EditorResources/EUW_NGuardian.EUW_NGuardian")



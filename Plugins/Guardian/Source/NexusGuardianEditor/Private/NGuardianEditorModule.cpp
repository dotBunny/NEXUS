// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NGuardianEditorModule.h"
#include "NEditorUtils.h"
#include "NGuardianEditorStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNGuardianEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNGuardianEditorModule, OnPostEngineInit);
}

void FNGuardianEditorModule::ShutdownModule()
{
	N_TOOLS_MENU_ENTRY_EUW_METHOD_UNREGISTER(EUW_NGuardian)();
	FNGuardianEditorStyle::Shutdown();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FNGuardianEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	// Configure Style
	FNGuardianEditorStyle::Initialize();
	
	// Initialize Tool Menu
	if (FSlateApplication::IsInitialized())
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(&N_TOOLS_MENU_ENTRY_EUW_METHOD_REGISTER(EUW_NGuardian)));
	}
}

IMPLEMENT_MODULE(FNGuardianEditorModule, NexusGuardianEditor)

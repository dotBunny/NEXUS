// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NGuardianEditorModule.h"
#include "NEditorUtils.h"
#include "NGuardianEditorStyle.h"
#include "NGuardianEditorToolMenu.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNGuardianEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNGuardianEditorModule, OnPostEngineInit);
}

void FNGuardianEditorModule::ShutdownModule()
{
	FNGuardianEditorToolMenu::Unregister();
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
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNGuardianEditorToolMenu::Register));
	}
}

IMPLEMENT_MODULE(FNGuardianEditorModule, NexusGuardianEditor)

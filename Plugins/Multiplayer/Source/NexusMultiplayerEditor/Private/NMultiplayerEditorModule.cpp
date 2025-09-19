// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMultiplayerEditorModule.h"

#include "NEditorUtils.h"
#include "NMultiplayerEditorStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "Macros/NModuleMacros.h"
#include "NMultiplayerEditorToolMenu.h"

void FNMultiplayerEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNMultiplayerEditorModule, OnPostEngineInit);
}

void FNMultiplayerEditorModule::ShutdownModule()
{
	FNMultiplayerEditorStyle::Shutdown();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FNMultiplayerEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	// Configure Style
	FNMultiplayerEditorStyle::Initialize();
	
	// Initialize Tool Menu
	if (FSlateApplication::IsInitialized())
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNMultiplayerEditorToolMenu::Register));
	}
}

IMPLEMENT_MODULE(FNMultiplayerEditorModule, NexusMultiplayerEditor)

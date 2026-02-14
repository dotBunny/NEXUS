// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NToolsEditorModule.h"

#include "NEditorUtils.h"
#include "NToolsEditorCommands.h"
#include "NToolsEditorStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNToolsEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNToolsEditorModule, OnPostEngineInit);
}


void FNToolsEditorModule::ShutdownModule()
{
	FNToolsEditorCommands::RemoveMenuEntries();
	FNToolsEditorStyle::Shutdown();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FNToolsEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	FNToolsEditorStyle::Initialize();
	
	// Initialize Tool Menu
	if (FSlateApplication::IsInitialized())
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNToolsEditorCommands::AddMenuEntries));
	}
}

IMPLEMENT_MODULE(FNToolsEditorModule, NexusToolsEditor)



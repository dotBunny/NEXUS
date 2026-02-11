// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NUIEditorModule.h"

#include "NEditorUtils.h"
#include "NUIEditorStyle.h"
#include "NUIEditorCommands.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNUIEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNUIEditorModule, OnPostEngineInit);
}


void FNUIEditorModule::ShutdownModule()
{
	FNUIEditorCommands::RemoveMenuEntries();
	FNUIEditorStyle::Shutdown();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FNUIEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	FNUIEditorStyle::Initialize();
	
	// Initialize Tool Menu
	if (FSlateApplication::IsInitialized())
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNUIEditorCommands::AddMenuEntries));
	}
}

IMPLEMENT_MODULE(FNUIEditorModule, NexusUIEditor)



// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NUIEditorModule.h"

#include "NEditorUtils.h"
#include "NUIEditorStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNUIEditorModule::StartupModule()
{
	N_MODULE_POST_ENGINE_INIT_STATIC(FNUIEditorModule::OnPostEngineInit);
}

void FNUIEditorModule::ShutdownModule()
{
	N_MODULE_REMOVE_POST_ENGINE_INIT_DELEGATE()
	FNUIEditorStyle::Shutdown();
}


void FNUIEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	FNUIEditorStyle::Initialize();
}

IMPLEMENT_MODULE(FNUIEditorModule, NexusUIEditor)
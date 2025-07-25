// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NUIEditorModule.h"
#include "NUIEditorStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNUIEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNUIEditorModule, OnPostEngineInit);
}

void FNUIEditorModule::ShutdownModule()
{
	FNUIEditorStyle::Shutdown();
}

void FNUIEditorModule::OnPostEngineInit()
{
	if (IsModuleInitialized()) return;
	FNUIEditorStyle::Initialize();
	bIsModuleInitialized = true;
}

IMPLEMENT_MODULE(FNUIEditorModule, NexusUIEditor)
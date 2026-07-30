// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NUIEditorModule.h"

#include "NEditorUtils.h"
#include "NPropertySections.h"
#include "NUIEditorStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"


N_MODULE_POST_ENGINE_INIT_STATIC_DELEGATE_IMPLEMENTATION(FNUIEditorModule)

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
	FNPropertySections::AddSceneComponentCategory("Text Render Component");
}

IMPLEMENT_MODULE(FNUIEditorModule, NexusUIEditor)
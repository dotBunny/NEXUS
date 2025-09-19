// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSharedSamplesEditorModule.h"

#include "NEditorUtils.h"
#include "NSharedSamplesEditorStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "Macros/NModuleMacros.h"

void FNSharedSamplesEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNSharedSamplesEditorModule, OnPostEngineInit);
}

void FNSharedSamplesEditorModule::ShutdownModule()
{
	FNSharedSamplesEditorStyle::Shutdown();
}

void FNSharedSamplesEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	// Configure Style
	FNSharedSamplesEditorStyle::Initialize();
}

IMPLEMENT_MODULE(FNSharedSamplesEditorModule, NexusSharedSamplesEditor)

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicReferencesEditorModule.h"
#include "NDynamicReferencesEditorStyle.h"
#include "NEditorUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNDynamicReferencesEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNDynamicReferencesEditorModule, OnPostEngineInit);
}

void FNDynamicReferencesEditorModule::ShutdownModule()
{
	FNDynamicReferencesEditorStyle::Shutdown();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FNDynamicReferencesEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	// Configure Style
	FNDynamicReferencesEditorStyle::Initialize();
}

IMPLEMENT_MODULE(FNDynamicReferencesEditorModule, NexusDynamicReferencesEditor)

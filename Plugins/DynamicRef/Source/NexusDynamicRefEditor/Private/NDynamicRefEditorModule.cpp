// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefEditorModule.h"
#include "NEditorUtils.h"
#include "NDynamicRefEditorStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNDynamicRefEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNDynamicRefEditorModule, OnPostEngineInit);
}

void FNDynamicRefEditorModule::ShutdownModule()
{
	FNDynamicRefEditorStyle::Shutdown();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FNDynamicRefEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	// Configure Style
	FNDynamicRefEditorStyle::Initialize();
}

IMPLEMENT_MODULE(FNDynamicRefEditorModule, NexusDynamicRefEditor)

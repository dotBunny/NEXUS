// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NUIEditorModule.h"

#include "NEditorUtilityWidgetLoadTask.h"
#include "NEditorUtils.h"
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

// ReSharper disable once CppMemberFunctionMayBeStatic
void FNUIEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	FNUIEditorStyle::Initialize();
	UNEditorUtilityWidgetLoadTask::Create();
}

IMPLEMENT_MODULE(FNUIEditorModule, NexusUIEditor)
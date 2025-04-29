// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NFixersEditorModule.h"

#include "NFixersEditorBulkOperations.h"
#include "NFixersEditorCommands.h"
#include "NFixersEditorStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNFixersEditorModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusFixers")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNFixersEditorModule, OnPostEngineInit);
}

void FNFixersEditorModule::ShutdownModule()
{
	FNFixersEditorStyle::Shutdown();
}

void FNFixersEditorModule::OnPostEngineInit()
{
	FNFixersEditorStyle::Initialize();
	
	if (FSlateApplication::IsInitialized())
	{
		FNFixersEditorCommands::Register();
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNFixersEditorBulkOperations::Register));
	}
}

IMPLEMENT_MODULE(FNFixersEditorModule, NexusFixersEditor)

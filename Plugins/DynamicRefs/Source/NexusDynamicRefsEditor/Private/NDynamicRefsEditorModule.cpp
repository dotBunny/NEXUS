// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefsEditorModule.h"
#include "NEditorUtils.h"
#include "NDynamicRefsEditorStyle.h"
#include "NDynamicRefsEditorToolMenu.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNDynamicRefsEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNDynamicRefsEditorModule, OnPostEngineInit);
}

void FNDynamicRefsEditorModule::ShutdownModule()
{
	FNDynamicRefsEditorStyle::Shutdown();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FNDynamicRefsEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	// Configure Style
	FNDynamicRefsEditorStyle::Initialize();
	
	// Initialize Tool Menu
	if (FSlateApplication::IsInitialized())
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNDynamicRefsEditorToolMenu::Register));
	}
}

IMPLEMENT_MODULE(FNDynamicRefsEditorModule, NexusDynamicRefsEditor)

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefsEditorModule.h"
#include "NEditorUtils.h"
#include "NDynamicRefsEditorStyle.h"
#include "NPropertySections.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNDynamicRefsEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT_STATIC(FNDynamicRefsEditorModule::OnPostEngineInit);
}

void FNDynamicRefsEditorModule::ShutdownModule()
{
	N_TOOLS_MENU_ENTRY_EUW_METHOD_UNREGISTER(EUW_NDynamicRefs)();
	FNDynamicRefsEditorStyle::Shutdown();
}

void FNDynamicRefsEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	// Configure Style
	FNDynamicRefsEditorStyle::Initialize();
	
	// Initialize Tool Menu
	if (FSlateApplication::IsInitialized())
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(&N_TOOLS_MENU_ENTRY_EUW_METHOD_REGISTER(EUW_NDynamicRefs)));
	}
	
	// Inspector Category Filter
	FNPropertySections::AddActorComponentCategory("DynamicRef");
}

IMPLEMENT_MODULE(FNDynamicRefsEditorModule, NexusDynamicRefsEditor)

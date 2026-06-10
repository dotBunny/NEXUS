// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCoreEditorModule.h"

#include "NCoreEditorMinimal.h"
#include "NEditorCommands.h"
#include "NEditorStyle.h"
#include "NEditorUtils.h"
#include "NPropertySections.h"
#include "DelayedEditorTasks/NUpdateCheckDelayedEditorTask.h"
#include "Modules/ModuleManager.h"

N_MODULE_POST_ENGINE_INIT_STATIC_DELEGATE_IMPLEMENTATION(FNCoreEditorModule)

void FNCoreEditorModule::StartupModule()
{
	if (IsRunningGame()) return;
	
	if (!FNEditorUtils::IsUserControlled())
	{
		UE_LOG(LogNexusCoreEditor, Log, TEXT("Framework initializing in an automated environment; some functionality will be ignored."));
	}
	
	N_MODULE_POST_ENGINE_INIT_STATIC(FNCoreEditorModule::OnPostEngineInit);
}

void FNCoreEditorModule::ShutdownModule()
{
	N_MODULE_REMOVE_POST_ENGINE_INIT_DELEGATE()
	UToolMenus::UnRegisterStartupCallback(this);
	FNEditorCommands::RemoveMenuEntries();
	FNEditorStyle::Shutdown();
	IModuleInterface::ShutdownModule();
}

void FNCoreEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;

	// Setup staging rules for configs
	FNEditorUtils::DisallowConfigFileFromStaging("DefaultNexusEditor");
	FNEditorUtils::AllowConfigFileForStaging("DefaultNexusGame");
	
	FNEditorStyle::Initialize();

	// Nothing works without the application being initialized
	if (!FSlateApplication::IsInitialized()) return;
	
	FNEditorCommands::Register();
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNEditorCommands::AddMenuEntries));
	
	// Initialize our simplified property section manager
	FNPropertySections::Register();
	
	// Start update check
	UNUpdateCheckDelayedEditorTask::Create();
}


IMPLEMENT_MODULE(FNCoreEditorModule, NexusCoreEditor)
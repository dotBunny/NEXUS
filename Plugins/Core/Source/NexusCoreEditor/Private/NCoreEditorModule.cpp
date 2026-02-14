// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCoreEditorModule.h"

#include "NCoreEditorMinimal.h"
#include "NEditorCommands.h"
#include "NEditorSettings.h"
#include "NEditorStyle.h"
#include "NEditorUserSettings.h"
#include "DelayedEditorTasks/NUpdateCheckDelayedEditorTask.h"
#include "Modules/ModuleManager.h"

void FNCoreEditorModule::StartupModule()
{
	if (IsRunningGame()) return;
	
	if (!FNEditorUtils::IsUserControlled())
	{
		UE_LOG(LogNexusCoreEditor, Log, TEXT("Framework initializing in an automated environment; some functionality will be ignored."));
	}
	
	UNEditorSettings::Register();
	UNEditorUserSettings::Register();

	
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNCoreEditorModule, OnPostEngineInit);
}

void FNCoreEditorModule::ShutdownModule()
{
	FNEditorCommands::RemoveMenuEntries();
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
	
	// Start update check
	UNUpdateCheckDelayedEditorTask::Create();
}


IMPLEMENT_MODULE(FNCoreEditorModule, NexusCoreEditor)
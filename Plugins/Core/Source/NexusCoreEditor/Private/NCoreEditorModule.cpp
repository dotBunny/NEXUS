// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCoreEditorModule.h"
#include "NCoreEditorMinimal.h"
#include "NEditorCommands.h"
#include "NEditorInputProcessor.h"
#include "NEditorSettings.h"
#include "NEditorStyle.h"
#include "NEditorUserSettings.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "NexusCoreEditor"

void FNCoreEditorModule::StartupModule()
{
	if (IsRunningGame()) return;
	
	if (!FNEditorUtils::IsUserControlled())
	{
		NE_LOG(Log, TEXT("[FNCoreEditorModule::StartupModule] Framework initializing in an automated environment; some functionality will be ignored."));
	}

	NE_LOG(Log, TEXT("[FNCoreEditorModule::StartupModule] Initializing ..."));
	
	UNEditorSettings::Register();
	UNEditorUserSettings::Register();

	if (!IsRunningCommandlet() && InputProcessor == nullptr)
	{
		InputProcessor = MakeShared<FNEditorInputProcessor>();
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
	}

	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNCoreEditorModule, OnPostEngineInit);
}

void FNCoreEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	UNEditorUserSettings::OnPostEngineInit();
	
	FNEditorStyle::Initialize();

	if (FSlateApplication::IsInitialized())
	{
		FNEditorCommands::Register();
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNEditorCommands::BuildMenus));
	}

}
#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNCoreEditorModule, NexusCoreEditor)
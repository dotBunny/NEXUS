// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCoreEditorModule.h"

#include "NCoreEditorMinimal.h"
#include "NEditorCommands.h"
#include "NEditorStyle.h"
#include "NEditorUtils.h"
#include "NPropertySections.h"
#include "DelayedEditorTasks/NUpdateCheckDelayedEditorTask.h"
#include "Editor/UnrealEdEngine.h"
#include "Modules/ModuleManager.h"
#include "PCG/NTargetPointComponent.h"
#include "UnrealEdGlobals.h"
#include "Visualizers/NTargetPointComponentVisualizer.h"

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
	// No-op in practice: the startup callback(s) are bound via CreateStatic (no `this` owner
	// for RemoveAll to match) and RegisterStartupCallback usually runs them immediately. Kept
	// for symmetry with Epic's module template; the real menu teardown is below.
	UToolMenus::UnRegisterStartupCallback(this);
	FNEditorCommands::RemoveMenuEntries();

	// Mirror the Register() in OnPostEngineInit. IsRegistered() skips the headless cook/commandlet path, where
	// Register() never ran (gated on IsUserControlled() + Slate init).
	if (FNEditorCommands::IsRegistered())
	{
		FNEditorCommands::Unregister();
	}

	// Mirror the FNPropertySections::Register() in OnPostEngineInit. Safe to call unconditionally: when Register()
	// never ran (headless cook/commandlet path), the cached pointers are null and the queues empty, so this is a no-op.
	FNPropertySections::Unregister();

	// Mirror the registration in OnPostEngineInit. Unregistering a visualizer that was never registered — the
	// automated path, where OnPostEngineInit returns before reaching it — is a no-op, so this is unconditional.
	if (GUnrealEd != nullptr)
	{
		GUnrealEd->UnregisterComponentVisualizer(UNTargetPointComponent::StaticClass()->GetFName());
	}

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

	// Visualizers. Kept as a local rather than a module member: it is only ever used through the base
	// FComponentVisualizer interface, and UnrealEd holds its own shared ref once registered.
	if (GUnrealEd != nullptr)
	{
		const TSharedPtr<FComponentVisualizer> TargetPointComponentVisualizer = MakeShared<FNTargetPointComponentVisualizer>();
		GUnrealEd->RegisterComponentVisualizer(UNTargetPointComponent::StaticClass()->GetFName(), TargetPointComponentVisualizer);
		TargetPointComponentVisualizer->OnRegister();
	}

	// Start update check
	UNUpdateCheckDelayedEditorTask::Create();
}


IMPLEMENT_MODULE(FNCoreEditorModule, NexusCoreEditor)
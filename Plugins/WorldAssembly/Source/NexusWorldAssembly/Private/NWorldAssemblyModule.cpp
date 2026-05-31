// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyModule.h"

#include "NWorldAssemblyRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNWorldAssemblyModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusWorldAssembly")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNWorldAssemblyModule, OnPostEngineInit);
}

void FNWorldAssemblyModule::ShutdownModule()
{
	if (PostWorldCleanupHandle.IsValid())
	{
		FWorldDelegates::OnPostWorldCleanup.Remove(PostWorldCleanupHandle);
		PostWorldCleanupHandle.Reset();
	}

	IModuleInterface::ShutdownModule();
}

void FNWorldAssemblyModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusWorldAssemblySamples")
	PostWorldCleanupHandle = FWorldDelegates::OnPostWorldCleanup.AddStatic(FNWorldAssemblyRegistry::OnPostWorldCleanup);
}

IMPLEMENT_MODULE(FNWorldAssemblyModule, NexusWorldAssembly)
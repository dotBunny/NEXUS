// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Logging/LogMacros.h"
#include "NCoreMinimal.h"

/**
 * Injects a strongly-typed Type& Get() accessor into an IModuleInterface subclass.
 *
 * @param Type Concrete module class, e.g. FNCoreModule.
 * @param Name Module name string, e.g. "NexusCore".
 */
#define N_IMPLEMENT_MODULE(Type, Name) \
public: \
	FORCEINLINE static Type& Get() \
	{ \
		return FModuleManager::LoadModuleChecked<Type>(Name); \
	} \

/**
 * Runs Method() now if the engine is already past PostDefault loading, otherwise defers to OnPostEngineInit.
 *
 * Useful from a plugin's StartupModule() to defer work that depends on other plugins/subsystems
 * that may not have loaded yet during early startup phases.
 *
 * @param Type Enclosing module class used when binding the delegate.
 * @param Method Member function to invoke once the engine has finished post-engine init.
 */
#define N_IMPLEMENT_MODULE_POST_ENGINE_INIT(Type, Method) \
	if (IPluginManager::Get().GetLastCompletedLoadingPhase() >= ELoadingPhase::PostDefault) \
	{ \
		Method(); \
	} \
	else \
	{ \
		FCoreDelegates::OnPostEngineInit.AddRaw(this, &Type::Method); \
	}

/**
 * Editor-only: synchronises a plugin's .uplugin Version / VersionName with NEXUS::Version.
 *
 * Called from StartupModule to keep every plugin's descriptor in lockstep with the framework
 * version. Compiles to nothing in non-editor builds.
 *
 * @param PluginName The string name of the plugin whose descriptor should be updated.
 */
// Only do version updates in the editor
#if WITH_EDITOR && WITH_EDITORONLY_DATA
#define N_UPDATE_UPLUGIN(PluginName) \
	if (const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName); Plugin.IsValid()) \
	{ \
		const FPluginDescriptor CurrentDescriptor = Plugin->GetDescriptor(); \
		FPluginDescriptor UpdatedDescriptor = FPluginDescriptor(CurrentDescriptor); \
		bool bIsDirty = false; \
		if(UpdatedDescriptor.Version != NEXUS::Version::Number) \
		{ \
			UpdatedDescriptor.Version = NEXUS::Version::Number; \
			bIsDirty = true; \
		} \
		FString NewVersion = FString::Printf(TEXT("%i.%i.%i"), NEXUS::Version::Major, NEXUS::Version::Minor, NEXUS::Version::Patch); \
		if(!UpdatedDescriptor.VersionName.Equals(NewVersion, ESearchCase::CaseSensitive)) \
		{ \
			UpdatedDescriptor.VersionName = NewVersion; \
			bIsDirty = true; \
		} \
		if(bIsDirty) \
		{ \
			UE_LOG(LogNexusCore, Verbose, TEXT("Updating plugin(%s) definition."), TEXT(PluginName)) \
			if (FText FailReason; !Plugin->UpdateDescriptor(UpdatedDescriptor, FailReason)) \
			{ \
				UE_LOG(LogNexusCore, Error, TEXT("Failed to update plugin(%s) descriptor: %s"), TEXT(PluginName), *FailReason.ToString()) \
			} \
		} \
	}
#else // !(WITH_EDITOR && WITH_EDITORONLY_DATA)
#define N_UPDATE_UPLUGIN(PluginName)
#endif // WITH_EDITOR && WITH_EDITORONLY_DATA

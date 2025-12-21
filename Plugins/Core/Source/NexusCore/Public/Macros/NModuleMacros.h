// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Logging/LogMacros.h"
#include "NCoreMinimal.h"

#define N_IMPLEMENT_MODULE(Type, Name) \
public: \
	FORCEINLINE static Type& Get() \
	{ \
		return FModuleManager::LoadModuleChecked<Type>(Name); \
	} \

#define N_IMPLEMENT_MODULE_POST_ENGINE_INIT(Type, Method) \
	if (IPluginManager::Get().GetLastCompletedLoadingPhase() >= ELoadingPhase::PostDefault) \
	{ \
		Method(); \
	} \
	else \
	{ \
		FCoreDelegates::OnPostEngineInit.AddRaw(this, &Type::Method); \
	}

// Only do version updates in the editor
#if WITH_EDITOR && WITH_EDITORONLY_DATA
#define N_UPDATE_UPLUGIN(PluginName) \
	if (const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName); Plugin.IsValid()) \
	{ \
		const FPluginDescriptor CurrentDescriptor = Plugin->GetDescriptor(); \
		FPluginDescriptor UpdatedDescriptor = FPluginDescriptor(CurrentDescriptor); \
		bool bIsDirty = false; \
		if(UpdatedDescriptor.Version != N_VERSION_NUMBER) \
		{ \
			UpdatedDescriptor.Version = N_VERSION_NUMBER; \
			bIsDirty = true; \
		} \
		FString NewVersion = FString::Printf(TEXT("%i.%i.%i"), N_VERSION_MAJOR, N_VERSION_MINOR, N_VERSION_PATCH); \
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
#else
#define N_UPDATE_UPLUGIN(PluginName)
#endif

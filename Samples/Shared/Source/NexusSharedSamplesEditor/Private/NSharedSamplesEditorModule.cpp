// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSharedSamplesEditorModule.h"

#include "IPlacementModeModule.h"
#include "NCoreEditorMinimal.h"
#include "NEditorDefaults.h"
#include "NEditorUtils.h"
#include "NSamplesDisplayActor.h"
#include "NSamplesLevelActor.h"
#include "NSamplesPawn.h"
#include "NSharedSamplesEditorStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Macros/NEditorModuleMacros.h"
#include "Modules/ModuleManager.h"
#include "Macros/NModuleMacros.h"

void FNSharedSamplesEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNSharedSamplesEditorModule, OnPostEngineInit);
}

void FNSharedSamplesEditorModule::ShutdownModule()
{
	N_IMPLEMENT_UNREGISTER_PLACEABLE_ACTORS(PlacementActors)
	
	FNSharedSamplesEditorStyle::Shutdown();
}

void FNSharedSamplesEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	// Configure Style
	FNSharedSamplesEditorStyle::Initialize();
	
	// Handle Placement Definitions
	if (const FPlacementCategoryInfo* Info = FNEditorDefaults::GetPlacementCategory())
	{
		PlacementActors.Add(IPlacementModeModule::Get().RegisterPlaceableItem(Info->UniqueHandle, MakeShared<FPlaceableItem>(
		*ANSamplesDisplayActor::StaticClass(),
		FAssetData(ANSamplesDisplayActor::StaticClass()),
		NAME_None,
		NAME_None,
		TOptional<FLinearColor>(),
		TOptional<int32>(),
		NSLOCTEXT("NexusSharedSamplesEditor", "NSamplesDisplayActorPlacement", "Samples Display Actor"))));
		
		PlacementActors.Add(IPlacementModeModule::Get().RegisterPlaceableItem(Info->UniqueHandle, MakeShared<FPlaceableItem>(
		*ANSamplesLevelActor::StaticClass(),
		FAssetData(ANSamplesLevelActor::StaticClass()),
		NAME_None,
		NAME_None,
		TOptional<FLinearColor>(),
		TOptional<int32>(),
		NSLOCTEXT("NexusSharedSamplesEditor", "NSamplesLevelActorPlacement", "Samples Level Actor"))));
	}
}

IMPLEMENT_MODULE(FNSharedSamplesEditorModule, NexusSharedSamplesEditor)

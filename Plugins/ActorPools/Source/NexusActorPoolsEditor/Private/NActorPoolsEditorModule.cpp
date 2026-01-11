// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolsEditorModule.h"
#include "ComponentVisualizer.h"
#include "IPlacementModeModule.h"
#include "NActorPoolsEditorMinimal.h"
#include "NActorPoolsEditorStyle.h"
#include "NActorPoolSpawnerComponent.h"
#include "NEditorDefaults.h"
#include "NEditorUtils.h"
#include "NKillZoneActor.h"
#include "NPooledActor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Macros/NModuleMacros.h"
#include "Interfaces/IPluginManager.h"
#include "Macros/NEditorModuleMacros.h"
#include "Modules/ModuleManager.h"
#include "Visualizers/NActorPoolSpawnerComponentVisualizer.h"

void FNActorPoolsEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNActorPoolsEditorModule, OnPostEngineInit);
}

void FNActorPoolsEditorModule::ShutdownModule()
{
	if (GUnrealEd)
	{
		GUnrealEd->UnregisterComponentVisualizer(UNActorPoolSpawnerComponent::StaticClass()->GetFName());
	}
	
	// Teardown Placement
	N_IMPLEMENT_UNREGISTER_PLACEABLE_ACTORS(PlacementActors)
	
	FNActorPoolsEditorStyle::Shutdown();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FNActorPoolsEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	FNActorPoolsEditorStyle::Initialize();
	
	if (GUnrealEd)
	{
		const TSharedPtr<FComponentVisualizer> ActorPoolSpawnerComponentVisualizer = MakeShared<FNActorPoolSpawnerComponentVisualizer>();
		GUnrealEd->RegisterComponentVisualizer(UNActorPoolSpawnerComponent::StaticClass()->GetFName(), ActorPoolSpawnerComponentVisualizer);
		ActorPoolSpawnerComponentVisualizer->OnRegister();
	}
	
	// Handle Placement Definitions
	if (const FPlacementCategoryInfo* Info = FNEditorDefaults::GetPlacementCategory())
	{
		PlacementActors.Add(IPlacementModeModule::Get().RegisterPlaceableItem(Info->UniqueHandle, MakeShared<FPlaceableItem>(
		*ANKillZoneActor::StaticClass(),
		FAssetData(ANKillZoneActor::StaticClass()),
		NAME_None,
		NAME_None,
		TOptional<FLinearColor>(),
		TOptional<int32>(),
		NSLOCTEXT("NexusActorPoolsEditor", "Placement_NKillZoneActor", "KillZone Actor"))));
		PlacementActors.Add(IPlacementModeModule::Get().RegisterPlaceableItem(Info->UniqueHandle, MakeShared<FPlaceableItem>(
			*ANPooledActor::StaticClass(),
			FAssetData(ANPooledActor::StaticClass()),
			NAME_None,
			NAME_None,
			TOptional<FLinearColor>(),
			TOptional<int32>(),
			NSLOCTEXT("NexusActorPoolsEditor", "Placement_NPooledActor", "Pooled Actor"))));
	}
}


IMPLEMENT_MODULE(FNActorPoolsEditorModule, NexusActorPoolsEditor)

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorUtils.h"
#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/SCS_Node.h"

USceneComponent* FNActorUtils::GetRootComponentFromDefaultObject(const TSubclassOf<AActor>& ActorClass)
{
	if (ActorClass == nullptr) return nullptr;

	// Walk the (possibly multi-level) Blueprint class chain, returning the first
	// USceneComponent template found on any SCS root node.
	for (UClass* Cursor = *ActorClass; Cursor; Cursor = Cursor->GetSuperClass())
	{
		const UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(Cursor);
		if (BPGC == nullptr) break; // hit the first native class — stop SCS walk

		const USimpleConstructionScript* SCS = BPGC->SimpleConstructionScript;
		if (SCS == nullptr) continue;

		for (const USCS_Node* Node : SCS->GetRootNodes())
		{
			if (Node == nullptr) continue;
			if (USceneComponent* SceneTemplate = Cast<USceneComponent>(Node->ComponentTemplate))
			{
				return SceneTemplate;
			}
		}
	}

	// Native fallback: read the CDO's root (set by the native constructor).
	const AActor* CDO = ActorClass->GetDefaultObject<AActor>();
	return CDO ? CDO->GetRootComponent() : nullptr;
}


TArray<AActor*> FNActorUtils::GetWorldActors(const UWorld* World, const FNWorldActorFilterSettings& Settings)
{
	TArray<AActor*> ReturnActors;
	if (World == nullptr) return MoveTemp(ReturnActors);

	const bool bHasExclusionFunctionRef = Settings.ExclusionFunction.IsSet();
	for (TActorIterator<AActor> WorldActorIterator(World); WorldActorIterator; ++WorldActorIterator)
	{
		AActor* Actor = *WorldActorIterator;
		
		// Ensure it is not on its way to being destroyed
		if (!Actor || !IsValid(Actor)) continue;
		
		if (Settings.bIncludePlayerStarts && Actor->IsA<APlayerStart>())
		{
			ReturnActors.Add(Actor);
			continue;
		}
		
		// Exclude editor only
		if (Settings.bExcludeEditorOnly && Actor->IsEditorOnly()) continue;
		
		// Exclude when collision is disabled per setting
		if (Settings.bExcludeNonCollisionEnabledActors && !Actor->GetActorEnableCollision()) continue;
		
		// Exclude because of filter
		if (bHasExclusionFunctionRef && !Settings.ExclusionFunction(Actor))
		{
			continue;
		}
		
		ReturnActors.Add(Actor);
	}
	
	return MoveTemp(ReturnActors);
}

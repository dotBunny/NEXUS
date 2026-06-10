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
	if (World == nullptr) return ReturnActors;

	for (TActorIterator<AActor> WorldActorIterator(World); WorldActorIterator; ++WorldActorIterator)
	{
		AActor* Actor = *WorldActorIterator;
		if (PassesFilter(Actor, Settings))
		{
			ReturnActors.Add(Actor);
		}
	}

	return ReturnActors;
}

bool FNActorUtils::PassesFilter(const AActor* Actor, const FNWorldActorFilterSettings& Settings)
{
	// Ensure it is not on its way to being destroyed
	if (!Actor || !IsValid(Actor)) return false;

	if (Settings.bIncludePlayerStarts && Actor->IsA<APlayerStart>()) return true;

	// Exclude editor only
	if (Settings.bExcludeEditorOnly && Actor->IsEditorOnly()) return false;

	// Exclude when collision is disabled per setting
	if (Settings.bExcludeNonCollisionEnabledActors && !Actor->GetActorEnableCollision()) return false;

	// Exclude because of filter
	if (Settings.ExclusionFunction.IsSet() && !Settings.ExclusionFunction(Actor)) return false;

	return true;
}

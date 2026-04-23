// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorUtils.h"
#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/SCS_Node.h"

USceneComponent* FNActorUtils::GetRootComponentFromDefaultObject(const TSubclassOf<AActor>& ActorClass)
{
	const UBlueprintGeneratedClass* BlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(ActorClass);
	
	if (BlueprintGeneratedClass == nullptr || BlueprintGeneratedClass->SimpleConstructionScript == nullptr)
	{
		return ActorClass->GetDefaultObject<AActor>()->GetRootComponent();
	}
	
	const TArray<USCS_Node*>& RootNodes =  BlueprintGeneratedClass->SimpleConstructionScript->GetRootNodes();
	if (RootNodes.Num() == 0 || RootNodes[0] == nullptr)
	{
		return ActorClass->GetDefaultObject<AActor>()->GetRootComponent();
	}
	
	
	UActorComponent* RootTemplate = RootNodes[0]->ComponentTemplate;
	if (RootTemplate->IsA<USceneComponent>())
	{
		return Cast<USceneComponent>(RootTemplate);
	}
	
	return ActorClass->GetDefaultObject<AActor>()->GetRootComponent();
}

TArray<AActor*> FNActorUtils::GetWorldActors(const UWorld* World, const FNWorldActorFilterSettings& Settings)
{
	TArray<AActor*> ReturnActors;
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

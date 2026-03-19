// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorUtils.h"
#include "CoreMinimal.h"
#include "Engine/SCS_Node.h"

USceneComponent* FNActorUtils::GetRootComponentFromDefaultObject(const TSubclassOf<AActor>& ActorClass)
{
	const UBlueprintGeneratedClass* BlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(ActorClass);
	if (BlueprintGeneratedClass != nullptr)
	{
		if ( BlueprintGeneratedClass->SimpleConstructionScript != nullptr)
		{
			const TArray<USCS_Node*>& RootNodes =  BlueprintGeneratedClass->SimpleConstructionScript->GetRootNodes();
			if (RootNodes.Num() > 0 && RootNodes[0] != nullptr)
			{
				UActorComponent* RootTemplate = RootNodes[0]->ComponentTemplate;
				if (RootTemplate->IsA<USceneComponent>())
				{
					return Cast<USceneComponent>(RootTemplate);
				}
			}
		}
	}
	
	return ActorClass->GetDefaultObject<AActor>()->GetRootComponent();
}
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorUtils.h"
#include "CoreMinimal.h"
#include "Engine/SCS_Node.h"

USceneComponent* FNActorUtils::GetRootComponentFromDefaultObject(const TSubclassOf<AActor>& ActorClass)
{
	if (UBlueprintGeneratedClass* BlueprintClass = Cast<UBlueprintGeneratedClass>(ActorClass))
	{
		TArray<const UBlueprintGeneratedClass*> BlueprintClasses;
		UBlueprintGeneratedClass::GetGeneratedClassesHierarchy(BlueprintClass, BlueprintClasses);
		for (const UBlueprintGeneratedClass* Class : BlueprintClasses)
		{
			if (Class->SimpleConstructionScript)
			{
				TArray<USCS_Node*> CDONodes = Class->SimpleConstructionScript->GetAllNodes();
				for (USCS_Node* Node : CDONodes)
				{
					UActorComponent* CDOComponent = Node->GetActualComponentTemplate(Cast<UBlueprintGeneratedClass>(BlueprintClass));
					if (USceneComponent* FoundSceneComponent = Cast<USceneComponent>(CDOComponent))
					{
						return FoundSceneComponent;
					}
				}
			}
		}
	}
	return ActorClass->GetDefaultObject<AActor>()->GetRootComponent();
}
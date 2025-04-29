// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "INActorPoolItem.h"
#include "NActorPoolSubsystem.h"
#include "NCoreMinimal.h"

void INActorPoolItem::InitializeActorPoolItem(FNActorPool* OwnerPool)
{
	bIsAttachedToActorPool = true;
	OwningActorPool = OwnerPool;
}

bool INActorPoolItem::IsAttachedToActorPool() const
{
	return bIsAttachedToActorPool;
}

bool INActorPoolItem::ReturnToActorPool()
{
	AActor* Actor = Cast<AActor>(this);
	if (Actor != nullptr)
	{
		if (OwningActorPool != nullptr)
		{
			return OwningActorPool->Return(Actor);
		}
		return Actor->GetWorld()->GetSubsystem<UNActorPoolSubsystem>()->ReturnActor(Actor);
	}
	N_LOG(Log, TEXT("[INActorPoolItem::ReturnToActorPool] Unable to return %s to a valid Actor Pool."), *Actor->GetName());
	return false;
}
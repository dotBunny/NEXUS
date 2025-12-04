// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "INActorPoolItem.h"
#include "NActorPoolSubsystem.h"
#include "NActorPool.h"

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
	UE_LOG(LogNexusActorPools, Warning, TEXT("Attempted to return a NULL or non-AActor to an FNActorPool."))
	return false;
}

bool INActorPoolItem::SetActorOperationalState(const ENActorOperationalState NewState)
{
	if (NewState == CurrentActorOperationalState)
	{
		return false;
	}
	
	PreviousActorOperationalState = CurrentActorOperationalState;
	CurrentActorOperationalState = NewState;
	
	OnActorOperationalStateChanged.Broadcast(PreviousActorOperationalState, CurrentActorOperationalState);
	return true;
}
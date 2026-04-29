// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NCreateVirtualWorldTask.h"
#include "Types/NRawMeshFactory.h"

void FNCreateVirtualWorldTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	N_PROC_GEN_ANALYTICS(CreateVirtualWorldContextStart)
	
	// Collect the world AActors that we need to care about
	const TArray<AActor*> WorldActors = FNActorUtils::GetWorldActors(VirtualWorldContextPtr->InputWorld, CreateWorldActorFilterSettings());
	
	// Gather simple-collision meshes from every primitive in the target world, restricted
	// to actors whose bounds fall inside one of the input organs' volume bounds.
	FNRawMeshFactory::FromActorsInBounds(WorldActors, VirtualWorldContextPtr->InputBounds,
		VirtualWorldContextPtr->WorldCollisionMeshes, VirtualWorldContextPtr->WorldCollisionMeshTransforms);

	N_PROC_GEN_ANALYTICS(CreateVirtualWorldContextFinish)
}
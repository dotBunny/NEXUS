// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NCreateVirtualWorldTask.h"
#include "Types/NRawMeshFactory.h"

void FNCreateVirtualWorldTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	N_ASSEMBLY_ANALYTICS(CreateVirtualWorldContextStart)

	// Collect the world AActors that we need to care about
	const TArray<AActor*> WorldActors = FNActorUtils::GetWorldActors(VirtualWorldContextPtr->InputWorld,
		CreateWorldActorFilterSettings(VirtualWorldContextPtr->WorldCollisionActorIgnoreTags));

	// Gather simple-collision meshes from every primitive in the target world, restricted
	// to actors whose bounds fall inside one of the input organs' volume bounds.
	FNRawMeshFactory::FromActorsInBounds(WorldActors,
		VirtualWorldContextPtr->InputBounds,
		VirtualWorldContextPtr->WorldCollisionMeshes,
		VirtualWorldContextPtr->WorldCollisionTransforms); // We'll bake the meshes off thread in the process phase

	N_ASSEMBLY_ANALYTICS(CreateVirtualWorldContextFinish)
}
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NCreateVirtualWorldTask.h"
#include "Types/NRawMeshFactory.h"

void FNCreateVirtualWorldTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	N_ASSEMBLY_ANALYTICS(CreateVirtualWorldContextStart)

	// Collect the world AActors that we need to care about
	const TArray<AActor*> WorldActors = FNActorUtils::GetWorldActors(VirtualWorldContextPtr->InputWorld,
		CreateWorldActorFilterSettings(VirtualWorldContextPtr->WorldCollisionSettings));

	// Gather simple-collision meshes from every primitive in the target world, restricted
	// to actors whose bounds fall inside one of the input organs' volume bounds.
	FNRawMeshFactory::FromActorsInBounds(WorldActors,
		VirtualWorldContextPtr->InputBounds,
		VirtualWorldContextPtr->WorldCollisionMeshes,
		VirtualWorldContextPtr->WorldCollisionTransforms); // We'll bake the meshes off thread in the process phase

	// Mesh terrain comes through the gather above like any other geometry, or not at all when the filter above refused
	// it. Landscape is neither: the factory skips landscape primitives because their collision is a heightfield behind
	// no UBodySetup, so without this an assembly sees no ground and routes cells straight through it. Sampling has to
	// happen here rather than in the process phase because it traces the live physics scene, which is game-thread only
	// — the same reason this task is.
	//
	// Deliberately unbounded, where the gather above is restricted to InputBounds: the organ volumes describe where
	// cells may go, not what they must clear on the way, and an assembly reads this world well outside them.
	if (VirtualWorldContextPtr->WorldCollisionSettings.bIncludeLandscapes)
	{
		FNRawMeshFactory::FromLandscapesInBounds(WorldActors, {},
			VirtualWorldContextPtr->WorldCollisionSettings.LandscapeSampleSpacing,
			VirtualWorldContextPtr->WorldCollisionMeshes,
			VirtualWorldContextPtr->WorldCollisionTransforms);
	}

	N_ASSEMBLY_ANALYTICS(CreateVirtualWorldContextFinish)
}
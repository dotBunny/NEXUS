// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NCreateVirtualWorldTask.h"
#include "NWorldAssemblyUtils.h"
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

	// Mesh terrain comes through the gather above like any other geometry. Landscape does not: the factory skips
	// landscape primitives because their collision is a heightfield behind no UBodySetup, so without this an assembly
	// sees no ground and routes cells straight through it. Sampling has to happen here rather than in the process
	// phase because it traces the live physics scene, which is game-thread only — the same reason this task is.
	if (const float SampleSpacing = VirtualWorldContextPtr->WorldCollisionSettings.LandscapeSampleSpacing; SampleSpacing > 0.f)
	{
		for (const AActor* Actor : WorldActors)
		{
			if (!FNActorUtils::IsLandscapeActor(Actor)) continue;

			FNRawMesh LandscapeMesh;
			if (!FNWorldAssemblyUtils::SampleLandscapeSurface(Actor, SampleSpacing, LandscapeMesh)) continue;

			// Sampled in world space already, so it pairs with an identity transform.
			VirtualWorldContextPtr->WorldCollisionMeshes.Add(MoveTemp(LandscapeMesh));
			VirtualWorldContextPtr->WorldCollisionTransforms.Add(FTransform::Identity);
		}
	}

	N_ASSEMBLY_ANALYTICS(CreateVirtualWorldContextFinish)
}
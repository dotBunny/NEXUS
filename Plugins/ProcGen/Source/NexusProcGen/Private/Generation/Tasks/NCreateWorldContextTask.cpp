// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NCreateWorldContextTask.h"
#include "Types/NRawMeshFactory.h"

void FNCreateWorldContextTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// Collect the world AActors that we need to care about
	const TArray<AActor*> WorldActors = FNActorUtils::GetWorldActors(WorldContextPtr->InputWorld, CreateWorldActorFilterSettings());
	
	// Gather simple-collision meshes from every primitive in the target world, restricted
	// to actors whose bounds fall inside one of the input organs' volume bounds.
	FNRawMeshFactory::FromActorsInBounds(WorldActors, WorldContextPtr->InputBounds,
		WorldContextPtr->CollisionMeshes, WorldContextPtr->CollisionMeshTransforms);
}
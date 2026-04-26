// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NCreateWorldTask.h"
#include "Types/NRawMeshFactory.h"

void FNCreateWorldTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// Collect the world AActors that we need to care about
	const TArray<AActor*> WorldActors = FNActorUtils::GetWorldActors(CreateWorldContextPtr->World, CreateWorldActorFilterSettings());
	
	// Gather simple-collision meshes from every primitive in the target world, restricted
	// to actors whose bounds fall inside one of the input organs' volume bounds.
	FNRawMeshFactory::FromActorsInBounds(WorldActors, CreateWorldContextPtr->Bounds,
		CreateWorldContextPtr->CollisionMeshes, CreateWorldContextPtr->CollisionMeshTransforms);
	
	
	// TODO: Actually split this based on the bounds index? so each volume will have its own collsion generated for it. Unbounded will get it all.
	// this way the actual context builders will have their immediate collision to worry about only
	
	// Need to actually do something when we are collecting organs, maybe key by name? 
}
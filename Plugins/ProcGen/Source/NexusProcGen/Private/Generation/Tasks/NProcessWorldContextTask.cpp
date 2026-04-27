// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NProcessWorldContextTask.h"

#include "Types/NRawMeshUtils.h"

void FNProcessWorldContextTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	const int32 MeshCount = WorldContextPtr->WorldCollisionMeshTransforms.Num();
	
	WorldContextPtr->WorldCollisionMeshLocations.Empty();
	WorldContextPtr->WorldCollisionMeshLocations.Reserve(MeshCount);
		
	WorldContextPtr->WorldCollisionMeshRotations.Empty();
	WorldContextPtr->WorldCollisionMeshRotations.Reserve(MeshCount);
		
	// We are going to massage the data a bit at this point:
	// - Apply each mesh's Transform scale against the vertices
	// - Cache the location and the rotation
	for (int32 i = 0; i < MeshCount; i++)
	{
		WorldContextPtr->WorldCollisionMeshes[i].ApplyScale(WorldContextPtr->WorldCollisionMeshTransforms[i]);
		WorldContextPtr->WorldCollisionMeshTransforms[i].SetScale3D(FVector::OneVector); // Clear Scale
		
		WorldContextPtr->WorldCollisionMeshLocations.Add(WorldContextPtr->WorldCollisionMeshTransforms[i].GetLocation());
		WorldContextPtr->WorldCollisionMeshRotations.Add(WorldContextPtr->WorldCollisionMeshTransforms[i].GetRotation().Rotator());
		
		// If the mesh is not convex were going to do it right here
		if (!WorldContextPtr->WorldCollisionMeshes[i].IsConvex())
		{
			WorldContextPtr->WorldCollisionMeshes[i] = FNRawMeshUtils::ToConvexHull(WorldContextPtr->WorldCollisionMeshes[i]);
		}
	}
}


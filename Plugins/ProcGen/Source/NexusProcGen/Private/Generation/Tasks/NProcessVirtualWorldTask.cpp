// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NProcessVirtualWorldTask.h"

#include "Types/NRawMeshUtils.h"

void FNProcessVirtualWorldTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	N_PROC_GEN_ANALYTICS(ProcessVirtualWorldContextStart)
	
	const int32 MeshCount = VirtualWorldContextPtr->WorldCollisionMeshTransforms.Num();
	
	// Ensure that we do not have any existing data
	VirtualWorldContextPtr->WorldCollisionMeshLocations.Empty();
	VirtualWorldContextPtr->WorldCollisionMeshLocations.Reserve(MeshCount);
	VirtualWorldContextPtr->WorldCollisionMeshRotations.Empty();
	VirtualWorldContextPtr->WorldCollisionMeshRotations.Reserve(MeshCount);
		
	// We are going to massage the data a bit at this point:
	// - Apply each mesh's Transform scale against the vertices
	// - Cache the location and the rotation
	for (int32 i = 0; i < MeshCount; i++)
	{
		VirtualWorldContextPtr->WorldCollisionMeshes[i].ApplyScale(VirtualWorldContextPtr->WorldCollisionMeshTransforms[i]);
		VirtualWorldContextPtr->WorldCollisionMeshTransforms[i].SetScale3D(FVector::OneVector); // Clear Scale
		VirtualWorldContextPtr->WorldCollisionMeshLocations.Add(VirtualWorldContextPtr->WorldCollisionMeshTransforms[i].GetLocation());
		VirtualWorldContextPtr->WorldCollisionMeshRotations.Add(VirtualWorldContextPtr->WorldCollisionMeshTransforms[i].GetRotation().Rotator());
		
		// If the mesh is not convex were going to do it right here
		if (!VirtualWorldContextPtr->WorldCollisionMeshes[i].IsConvex())
		{
			VirtualWorldContextPtr->WorldCollisionMeshes[i] = FNRawMeshUtils::ToConvexHull(VirtualWorldContextPtr->WorldCollisionMeshes[i]);
		}
	}
	
	N_PROC_GEN_ANALYTICS(ProcessVirtualWorldContextFinish)
}


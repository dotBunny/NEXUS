// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NProcessVirtualWorldTask.h"

#include "Types/NRawMeshUtils.h"

void FNProcessVirtualWorldTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	N_ASSEMBLY_ANALYTICS(ProcessVirtualWorldContextStart)
	
	const int32 MeshCount = VirtualWorldContextPtr->WorldCollisionMeshes.Num();
		
	// We are going to massage the data a bit at this point:
	// - Apply each mesh's Transform scale against the vertices
	// - Cache the location and the rotation
	for (int32 i = 0; i < MeshCount; i++)
	{
		// Unwind Transform
		VirtualWorldContextPtr->WorldCollisionMeshes[i].ApplyTransform(VirtualWorldContextPtr->WorldCollisionTransforms[i]);
		
		// If the mesh is not convex were going to do it right here
		if (!VirtualWorldContextPtr->WorldCollisionMeshes[i].IsConvex())
		{
			VirtualWorldContextPtr->WorldCollisionMeshes[i] = FNRawMeshUtils::ToConvexHull(VirtualWorldContextPtr->WorldCollisionMeshes[i]);
		}
	}
	
	// No point keeping this around
	VirtualWorldContextPtr->WorldCollisionTransforms.Empty();
	
	N_ASSEMBLY_ANALYTICS(ProcessVirtualWorldContextFinish)
}


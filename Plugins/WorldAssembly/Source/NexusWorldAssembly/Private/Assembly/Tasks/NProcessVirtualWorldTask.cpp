// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NProcessVirtualWorldTask.h"

#include "NWorldAssemblyMinimal.h"
#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
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

		// Warm the face-plane cache now, off the builder thread. Each FNOrganGraphBuilderTask copies these
		// meshes, so a cache built here propagates through the copy and spares every organ a lazy rebuild on
		// its first intersection query.
		VirtualWorldContextPtr->WorldCollisionMeshes[i].EnsureCachedFacePlanes();
	}

	// No point keeping this around
	VirtualWorldContextPtr->WorldCollisionTransforms.Empty();

	// This task gates the first pass of organ builders, so its completion marks the start of organ building.
	TaskGraphContextPtr->SetStatusMessage(NEXUS::WorldAssembly::StatusMessage::BuildingOrgans);

	N_ASSEMBLY_ANALYTICS(ProcessVirtualWorldContextFinish)
}


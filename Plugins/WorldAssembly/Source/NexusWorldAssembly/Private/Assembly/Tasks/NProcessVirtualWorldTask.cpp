// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NProcessVirtualWorldTask.h"

#include "NWorldAssemblyMinimal.h"
#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
#include "Math/NBoundsBVH.h"
#include "NWorldCollisionBaker.h"
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
		// Unwind the transform, force convexity, and warm the face-plane cache — the last of which matters here
		// because each FNOrganGraphBuilderTask copies these meshes, so a cache built now propagates through the copy
		// and spares every organ a lazy rebuild on its first intersection query.
		//
		// Shared with the collision cache's bake and the editor's collision visualizer rather than written out here.
		// This loop was the original definition of "what an assembly collides with", and while it was the only one,
		// the visualizer drifted into drawing un-convexified geometry that no collision test ever honoured.
		FNWorldCollisionBaker::BakeElement(VirtualWorldContextPtr->WorldCollisionMeshes[i],
			VirtualWorldContextPtr->WorldCollisionTransforms[i]);
	}

	// Fold in whatever the capture phase resolved from the level's collision cache. These were baked — transform
	// applied, hull made convex, face planes warmed — at the time they were cached, so they join the array already
	// in the form the loop above has just produced for everything else.
	//
	// Appended here rather than in the capture phase so this task's loop does not re-bake them, and before the
	// broadphase below so a builder's mesh indices cover the whole set and stay stable for the rest of the run.
	if (!VirtualWorldContextPtr->CachedWorldCollisionMeshes.IsEmpty())
	{
		VirtualWorldContextPtr->WorldCollisionMeshes.Append(MoveTemp(VirtualWorldContextPtr->CachedWorldCollisionMeshes));
		VirtualWorldContextPtr->CachedWorldCollisionMeshes.Reset();
	}

	// Build the broadphase now that every mesh is baked into world space, so its bounds are final. Built once here
	// and never mutated again, which is what lets every organ builder in every pass query it concurrently.
	{
		// Counted afresh rather than reusing MeshCount, which was taken before the cached hulls were appended and
		// covers only the gathered ones. Indexing the broadphase with it would leave every cached hull out of the
		// tree while the meshes themselves stayed in the array — geometry present but never queried.
		const int32 TotalMeshCount = VirtualWorldContextPtr->WorldCollisionMeshes.Num();

		TArray<FBox> MeshBounds;
		MeshBounds.Reserve(TotalMeshCount);
		for (int32 i = 0; i < TotalMeshCount; i++)
		{
			const FNRawMesh& Mesh = VirtualWorldContextPtr->WorldCollisionMeshes[i];
			const bool bHasBounds = Mesh.HasBounds();
			MeshBounds.Add(bHasBounds ? Mesh.Bounds : FBox(ForceInit));

			// A mesh without bounds gets no AABB rejection inside GetIntersectDepth, so it cannot be excluded by a
			// broadphase without changing the answer. Record it to be tested unconditionally instead.
			if (!bHasBounds)
			{
				VirtualWorldContextPtr->UnboundedWorldCollisionIndices.Add(i);
			}
		}
		VirtualWorldContextPtr->WorldCollisionBVH = FNBoundsBVH(MeshBounds);
	}

	// No point keeping this around
	VirtualWorldContextPtr->WorldCollisionTransforms.Empty();

	// This task gates the first pass of organ builders, so its completion marks the start of organ building.
	TaskGraphContextPtr->SetStatusMessage(NEXUS::WorldAssembly::StatusMessage::BuildingOrgans);

	N_ASSEMBLY_ANALYTICS(ProcessVirtualWorldContextFinish)
}


// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRawMesh.h"
#include "Developer/NDebugActor.h"
#include "Templates/Function.h"

/**
 * Intersection and containment helpers for FNRawMesh geometry.
 */
class NEXUSCORE_API FNRawMeshUtils
{
public:

	/**
	 * Appends OtherMesh into BaseMesh, re-expressing every Other vertex in BaseMesh's local space so the merged
	 * geometry preserves the world-space positional relationship between the two inputs.
	 *
	 * Each Other vertex is transformed Other-local → world (via OtherTransform) → Base-local (via BaseTransform's
	 * inverse) before being appended. Other's loop indices are then shifted past BaseMesh's existing vertex count
	 * and added to BaseMesh.Loops. BaseTransform itself is unchanged; the merged mesh remains anchored to it.
	 *
	 * @param BaseTransform World transform that defines BaseMesh's local frame; used to map Other's world-space verts back into Base-local.
	 * @param BaseMesh Mesh that receives the merged geometry. Center/Bounds are recomputed and Validate() is re-run on completion.
	 * @param OtherTransform World transform that defines OtherMesh's local frame.
	 * @param OtherMesh Source mesh to merge in. Left untouched (data is copied, not moved). No-op when it has no vertices or loops.
	 * @note BaseMesh's bIsChaosGenerated flag is cleared since the merged result is no longer a single cooked Chaos body.
	 */
	static void CombineMesh(const FTransform& BaseTransform, FNRawMesh& BaseMesh,
		const FTransform& OtherTransform, const FNRawMesh& OtherMesh);

	/**
	 * Tests whether two raw meshes intersect when placed at the given origins and rotations.
	 * Uses a transformed-AABB early-out, then a triangle-vs-triangle sweep, then a containment fallback so a
	 * fully-enclosed convex mesh is still reported as intersecting.
	 * @param LeftMesh First mesh.
	 * @param LeftOrigin World-space translation applied to LeftMesh.
	 * @param LeftRotation World-space rotation applied to LeftMesh.
	 * @param RightMesh Second mesh.
	 * @param RightOrigin World-space translation applied to RightMesh.
	 * @param RightRotation World-space rotation applied to RightMesh.
	 * @return true when the transformed meshes overlap.
	 * @note Returns false (with a log warning) when either mesh has zero loops, or (with a log error) when either mesh contains non-triangle loops.
	 */
	static bool DoesIntersect(
		const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
		const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation);

	/**
	 * Spawns transient ANDebugActor instances that render the supplied FNRawMesh entries in-world for visual diagnostics.
	 *
	 * Two output modes are supported:
	 * - **Per-mesh (default)**: one ANDebugActor per entry, each spawned at its matching Transforms[i].
	 * - **Single-actor (bSingleActor=true)**: every mesh is merged via CombineMesh into a single FNRawMesh anchored at the
	 *   world origin (identity transform), then visualized as one actor. Useful for treating a heterogeneous set as one
	 *   inspectable object.
	 *
	 * Each emitted FNRawMesh is converted via FNRawMesh::CreateDynamicMesh(bProcessMeshes) and assigned to the actor's
	 * dynamic-mesh override along with MaterialInterface. Spawned actors are flagged RF_Transient and given unique names
	 * prefixed with "RawMeshVisualizer".
	 *
	 * @param World World to spawn the debug actors into. Must be valid.
	 * @param Meshes Source meshes to visualize. Must be the same length as Transforms; Meshes[i] is paired with Transforms[i].
	 * @param Transforms World-space transform for each mesh entry. In single-actor mode these are used to align the meshes
	 *                   during the merge, not to position the resulting actor.
	 * @param MaterialInterface Material applied to every spawned actor's dynamic mesh. May be null.
	 * @param bSingleActor When true, merges all inputs into one FNRawMesh and spawns a single actor instead of one per mesh.
	 * @param bProcessMeshes Forwarded to FNRawMesh::CreateDynamicMesh — when true, runs triangle-reorientation and normal recomputation cleanup.
	 * @return The spawned debug actors, in spawn order. Length is Transforms.Num() in per-mesh mode, or 1 in single-actor mode.
	 * @note Intended for editor / developer diagnostics only. The actors are transient and will not be saved with the level.
	 */
	static TArray<ANDebugActor*> CreateRawMeshVisualizers(UWorld* World, TArray<FNRawMesh>& Meshes, const TArray<FTransform>& Transforms,
		UMaterialInterface* MaterialInterface, bool bSingleActor = false, bool bProcessMeshes = false);
	
	/**
	 * Tests whether RelativePoint lies inside Mesh using the mesh's local space.
	 * Implemented as a half-space test against every triangle plane — the point must lie on the same side
	 * of each plane as the mesh's pre-computed Center.
	 * @param Mesh The mesh to test against. Must be convex and triangle-based.
	 * @param RelativePoint Point expressed in Mesh's local space.
	 * @return true when RelativePoint is inside the mesh volume; false when the mesh is not convex, contains non-triangle loops, or the point is outside.
	 */
	static bool IsRelativePointInside(const FNRawMesh& Mesh, const FVector& RelativePoint);

	/**
	 * Tests whether any of RelativePoints are inside Mesh. Short-circuits on the first hit.
	 * @param Mesh The mesh to test against. Must be convex and triangle-based.
	 * @param RelativePoints Points expressed in Mesh's local space.
	 * @return true when at least one point lies inside the mesh; false when the mesh is not convex, contains non-triangle loops, or no point is inside.
	 */
	static bool AnyRelativePointsInside(const FNRawMesh& Mesh, const TArray<FVector>& RelativePoints);

private:
	
	static bool DoesIntersectTriangles(
		const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
		const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation);
};
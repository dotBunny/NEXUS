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
	 * fully-enclosed mesh is still reported as intersecting. Convex inputs use a face-plane containment test;
	 * non-convex inputs fall back to an odd-parity ray cast (see IsRelativePointInside).
	 * @remark This assumes that the vertices in the FNRawMesh's are already in position and are not effected by a Transform's scale.
	 * @remark Containment fallback assumes each mesh is a closed manifold (no open boundaries). Open / shell meshes
	 *         will give undefined containment results — surface-crossing detection still works.
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
	
	static bool DoesIntersect(const FNRawMesh& LeftMesh, const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation);
	
	static bool DoesIntersect(const FNRawMesh& LeftMesh, const FNRawMesh& RightMesh);

	/**
	 * Measures the maximum penetration depth between two meshes placed at the given origins and rotations.
	 *
	 * Symmetric vertex-sampling metric: returns the largest of
	 * - max distance from any RightMesh vertex (inside LeftMesh) to LeftMesh's nearest surface, and
	 * - max distance from any LeftMesh vertex (inside RightMesh) to RightMesh's nearest surface.
	 *
	 * Captures both "right poked into left" and "right engulfed a corner of left" with a single value.
	 *
	 * Convex inputs use a fast face-plane distance metric (O(faces) per query). Non-convex inputs fall back to
	 * a parity ray-cast containment probe plus point-to-triangle surface distance — ~2× the work per vertex.
	 *
	 * @param LeftMesh First mesh. Must be triangle-based; for non-convex inputs, must also be a closed manifold.
	 * @param LeftOrigin World-space translation applied to LeftMesh.
	 * @param LeftRotation World-space rotation applied to LeftMesh.
	 * @param RightMesh Second mesh. Same triangle / closed-manifold requirement as LeftMesh.
	 * @param RightOrigin World-space translation applied to RightMesh.
	 * @param RightRotation World-space rotation applied to RightMesh.
	 * @param EarlyExitDepth Optional optimization hint — when finite, the function may return as soon as it
	 *        proves the depth either exceeds or is bounded below this value. Callers doing a threshold
	 *        comparison (`if (depth >= threshold)`) should pass their threshold here; the returned value
	 *        still produces the correct branch on that compare, but may be a conservative upper bound
	 *        (when the shortcut proves "depth is bounded below threshold") rather than the exact maximum.
	 *        Default MAX_flt disables every shortcut and returns the exact maximum.
	 * @return The deepest measured penetration distance in mesh units, or -1.0 when the transformed AABBs do not
	 *         overlap, when either mesh has zero loops (logs a warning), or when either mesh contains non-triangle
	 *         loops (logs an error). Returns 0.0 when the AABBs overlap but no vertex of either mesh lies inside
	 *         the other (surface-only crossings are not measured by this metric).
	 * @remark This assumes that the vertices in the FNRawMesh's are already in position and are not effected by a Transform's scale.
	 * @note Intended for "max-allowed-penetration" threshold checks while iterating many meshes. For exact
	 *       boolean overlap including surface-only crossings without vertex containment, use DoesIntersect.
	 * @note On non-convex meshes "depth" is distance to the nearest triangle surface, not to the AABB. A vertex
	 *       deep inside a thin "arm" of the body reports a small depth because a nearby wall is close, even though
	 *       the AABB extends well past it — this is the correct geometric answer but can surprise callers who
	 *       expect AABB-relative numbers.
	 */
	static float GetIntersectDepth(
		const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
		const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation,
		float EarlyExitDepth = MAX_flt);
	
	static float GetIntersectDepth(const FNRawMesh& LeftMesh, const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation, float EarlyExitDepth = MAX_flt);
	
	static float GetIntersectDepth(const FNRawMesh& LeftMesh, const FNRawMesh& RightMesh, float EarlyExitDepth = MAX_flt);
	
	
	static float GetIntersectDepth(
		const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
		const FVector& WorldPosition, 
		float EarlyExitDepth = MAX_flt);
	
	static float GetIntersectDepth(const FNRawMesh& LeftMesh, const FVector& WorldPosition, float EarlyExitDepth = MAX_flt);


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
	static TArray<ANDebugActor*> CreateRawMeshVisualizers(UWorld* World, const TArray<FNRawMesh>& Meshes, const TArray<FTransform>& Transforms,
		UMaterialInterface* MaterialInterface, bool bSingleActor = false, bool bProcessMeshes = false);

	/**
	 * Builds a convex hull from the supplied mesh's vertex cloud using Chaos's hull builder.
	 * The source mesh's loop topology is discarded; only its vertices contribute to the hull.
	 * @param Mesh Source mesh whose vertices form the input point cloud.
	 * @return A new convex-hull mesh in the source's local space, triangulated when the builder emits n-gon faces.
	 *         When the source is already convex it is returned as-is (with a log warning) and no rebuild is performed.
	 *         An empty mesh is returned (with a log warning) when the source has fewer than four vertices or the builder produces no geometry.
	 * @note The result is flagged Chaos-generated and convex; bounds and center are computed directly from the hull output.
	 */
	static FNRawMesh ToConvexHull(const FNRawMesh& Mesh);

	/**
	 * Builds a valid, outward-wound convex box hull spanning the axis-aligned Box.
	 *
	 * Emits a hull shaped like the ones production paths produce (e.g. FNWorldAssemblyUtils::CalculateConvexHull):
	 * both a polygonal FaceLoops description and a matching triangulated Loops buffer, plus populated Center/Bounds.
	 * Every face is wound so its normal points away from the box interior, which is what the plane-side convexity
	 * test in FNRawMesh::CheckConvex requires (all other vertices must sit on or behind each face plane).
	 *
	 * @param Box Axis-aligned bounds the hull should span.
	 * @return A convex box hull with 8 vertices, 6 quad FaceLoops, 12 triangle Loops, populated Center/Bounds,
	 *         and validated convexity flags (IsConvex() reports true without further mutation).
	 */
	static FNRawMesh MakeBoxHull(const FBox& Box);

	/**
	 * Tests whether RelativePoint lies inside Mesh using the mesh's local space.
	 *
	 * For convex meshes (Mesh.IsConvex() == true), implemented as a half-space test against every triangle
	 * plane — the point must lie on the same side of each plane as the mesh's pre-computed Center. Exact
	 * and O(faces) per query.
	 *
	 * For non-convex meshes, falls back to a Möller-Trumbore odd-parity ray cast. A probe ray is cast in
	 * a deliberately non-axis-aligned direction; the point is inside iff the ray crosses an odd number of
	 * triangle interiors. The mesh must be a closed manifold for the parity to be meaningful — open / shell
	 * meshes give undefined results.
	 *
	 * @param Mesh The mesh to test against. Must be triangle-based (n-gon FaceLoops are not supported).
	 * @param RelativePoint Point expressed in Mesh's local space.
	 * @return true when RelativePoint is inside the mesh volume; false when the mesh contains non-triangle
	 *         loops (logs a warning) or the point is outside.
	 */
	static bool IsRelativePointInside(const FNRawMesh& Mesh, const FVector& RelativePoint);

	/**
	 * Tests whether any of RelativePoints are inside Mesh. Short-circuits on the first hit.
	 * @param Mesh The mesh to test against. Must be triangle-based. Convex and non-convex meshes are
	 *             both supported — see IsRelativePointInside for the per-point algorithm and the
	 *             closed-manifold requirement on non-convex inputs.
	 * @param RelativePoints Points expressed in Mesh's local space.
	 * @return true when at least one point lies inside the mesh; false when the mesh contains non-triangle
	 *         loops or no point is inside.
	 */
	static bool AnyRelativePointsInside(const FNRawMesh& Mesh, const TArray<FVector>& RelativePoints);

private:
	/** Dispatch wrapper: picks the convex fast path or the non-convex containment + surface-distance path. */
	static float ComputePointDepthInside(const FNRawMesh& Mesh, const FVector& RelativePoint);
	/** Convex fast path: max perpendicular distance to any face plane. Returns -1 when outside, depth when inside. */
	static float ComputePointDepthInsideConvex(const FNRawMesh& Mesh, const FVector& RelativePoint);
	/** Non-convex distance-to-surface: caller has already verified the point is inside; returns min Euclidean distance to any triangle. */
	static float ComputePointDepthInsideNonConvex(const FNRawMesh& Mesh, const FVector& RelativePoint);
	static bool DoesIntersectTriangles(
		const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
		const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation);

	/**
	 * Baked variant of DoesIntersectTriangles. Both meshes' vertices (and Bounds) are already in world
	 * space, so the per-vertex transform and the two heap-allocated world-vertex buffers the transform-aware
	 * version builds are skipped — the mesh vertices are referenced directly. Bit-identical to calling the
	 * transform-aware overload with identity transforms.
	 */
	static bool DoesIntersectTrianglesBaked(const FNRawMesh& LeftMesh, const FNRawMesh& RightMesh);


	/**
	 * Hybrid convex-vs-convex overlap test. First runs a SAT-style face-normal rejection pass that
	 * cheaply proves non-overlap when any face plane separates the two meshes. When no face normal
	 * separates, falls through to the standard tri-tri sweep (DoesIntersectTriangles), which
	 * terminates on the first surface hit and so beats SAT's O(E_L * E_R) edge-cross loop on the
	 * actual-overlap case.
	 *
	 * Why this is safe: the only convex-vs-convex configuration where face normals fail to separate
	 * AND tri-tri finds no surface intersection is "fully enclosed" (handled by the containment
	 * fallback inside DoesIntersectTriangles) or an exotic edge-on-edge near-touch (where tri-tri
	 * correctly returns no intersection and we report no overlap). Pure-SAT edge-cross axes can only
	 * be the deciding factor in that narrow corner — not worth their cost on the dominant hit path.
	 *
	 * Requires both meshes to be flagged convex AND carry a polygonal FaceLoops description. The
	 * CanUseSAT gate enforces both preconditions plus a face-count cap.
	 */
	static bool DoesConvexIntersectSAT(
		const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
		const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation);

	/**
	 * Baked variant of DoesConvexIntersectSAT. Both meshes' vertices are already world-space, so face
	 * normals and projection intervals are computed straight from the stored vertices with no rotation and
	 * no world-vertex buffers. Bit-identical to the transform-aware overload with identity transforms.
	 */
	static bool DoesConvexIntersectSATBaked(const FNRawMesh& LeftMesh, const FNRawMesh& RightMesh);

	/**
	 * Gate for the SAT fast path: both inputs must be convex with a populated FaceLoops description and
	 * face count under the per-mesh threshold. Threshold is empirical — the SAT hit-path cost grows as
	 * O(F^2) on edge crosses, so the cap keeps the worst case within striking distance of the tri-tri
	 * sweep while still catching every primitive / Chaos-built cell hull (boxes, capsules, spheres of
	 * modest resolution, artist hulls).
	 */
	static bool CanUseSAT(const FNRawMesh& LeftMesh, const FNRawMesh& RightMesh);
};
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRawMesh.h"

/**
 * Intersection and containment helpers for FNRawMesh geometry.
 */
class NEXUSCORE_API FNRawMeshUtils
{
public:
	/**
	 * Tests whether two raw meshes intersect when placed at the given origins and rotations.
	 * @param LeftMesh First mesh.
	 * @param LeftOrigin World-space translation applied to LeftMesh.
	 * @param LeftRotation World-space rotation applied to LeftMesh.
	 * @param RightMesh Second mesh.
	 * @param RightOrigin World-space translation applied to RightMesh.
	 * @param RightRotation World-space rotation applied to RightMesh.
	 * @return true when the transformed meshes overlap.
	 */
	static bool DoesIntersect(
		const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
		const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation);

	/**
	 * Walks every registered UPrimitiveComponent in World and extracts its simple-collision
	 * representation as FNRawMesh entries, along with parallel world-space transforms.
	 * - Only actors whose world-space bounds are fully contained within at least one of the
	 *   supplied ContainingBounds are processed. Passing an empty array will skip the test entirely.
	 * - Aggregate-geometry path covers FKConvexElem, FKBoxElem, FKSphereElem and FKSphylElem (capsule).
	 * - Complex-as-simple falls back to the source static-mesh render data, or Chaos TriMeshGeometries.
	 * - UInstancedStaticMeshComponent emits one FNRawMesh entry per instance.
	 * - Landscape-based primitives are skipped.
	 * @param World Source world to scan.
	 * @param ContainingBounds Actor-bounds filter; an actor is processed when its bounds are contained within any one of these. Check is skipped if array is empty.
	 * @param OutMeshes Each mesh in element-local space, appended to the array.
	 * @param OutTransforms Matching world-space transform per entry in OutMeshes.
	 * @note In editor builds, force-flushes any async static-mesh compilation so every UBodySetup is populated before reading.
	 */
	static void GetWorldSimpleCollisionMeshes(const UWorld* World, const TArray<FBoxSphereBounds>& ContainingBounds, TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms);

	/**
	 * Tests whether RelativePoint lies inside Mesh using the mesh's local space.
	 * @param Mesh The mesh to test against.
	 * @param RelativePoint Point expressed in Mesh's local space.
	 * @return true when RelativePoint is inside the mesh volume.
	 */
	static bool IsRelativePointInside(const FNRawMesh& Mesh, const FVector& RelativePoint);

	/**
	 * Tests whether any of RelativePoints are inside Mesh.
	 * @param Mesh The mesh to test against.
	 * @param RelativePoints Points expressed in Mesh's local space.
	 * @return true when at least one point lies inside the mesh.
	 */
	static bool AnyRelativePointsInside(const FNRawMesh& Mesh, const TArray<FVector>& RelativePoints);

private:

	/**
	 * Dispatches an FKAggregateGeom's convex/box/sphere/capsule elements through the per-element appenders.
	 * @param Agg Aggregate geometry block (typically UBodySetup::AggGeom).
	 * @param BaseToWorld Transform applied on top of each element's local transform before emitting.
	 * @param OutMeshes Destination for appended meshes.
	 * @param OutTransforms Destination for appended transforms, parallel to OutMeshes.
	 */
	static void AppendChaosAggregateGeometry(const FKAggregateGeom& Agg, const FTransform& BaseToWorld,
		TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms);
	
	/**
	 * Class-name heuristic to skip landscape primitives without taking a hard Landscape-module dependency.
	 * @param Prim Primitive component to inspect.
	 * @return true when the component's class name begins with "Landscape".
	 */
	static bool IsLandscapePrimitive(const UPrimitiveComponent* Prim);
	
	static bool DoesIntersectTriangles(
		const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
		const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation);
};
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRawMesh.h"
#include "PhysicsEngine/ConvexElem.h"
#include "PhysicsEngine/SphereElem.h"
#include "PhysicsEngine/SphylElem.h"

class UWorld;
class UStaticMesh;
class UBodySetup;
class UPrimitiveComponent;

// TODO: Add landscape support
// TODO: Move all of the creation to constructors of FNRawMesh

/**
 * Helpers for extracting the simple-collision geometry of every primitive in a UWorld
 * as FNRawMesh entries (plus matching world-space transforms).
 *
 * Consumers iterate the resulting parallel arrays to reason about world obstructions
 * without having to touch UBodySetup / Chaos collision types directly.
 */
class NEXUSCORE_API FNRawMeshWorldUtils
{
public:
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
	 * Emits an FKBoxElem as an 8-vertex / 12-triangle FNRawMesh. The element's Center and Rotation are
	 * folded into the emitted transform; the mesh itself is box-half-extent-local.
	 * @param Box Box element to emit.
	 * @param CompToWorld Owning component's component-to-world transform.
	 * @param OutMeshes Destination for the appended mesh.
	 * @param OutTransforms Destination for the appended transform.
	 */
	static void AppendChaosBox(const FKBoxElem& Box, const FTransform& CompToWorld,
		TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms);

	/**
	 * Tessellates an FKSphylElem as a cylindrical band plus two hemispherical caps along the element's local Z axis.
	 * The element's Center and Rotation are folded into the emitted transform.
	 * @param Capsule Capsule (sphyl) element to emit.
	 * @param CompToWorld Owning component's component-to-world transform.
	 * @param OutMeshes Destination for the appended mesh.
	 * @param OutTransforms Destination for the appended transform.
	 */
	static void AppendChaosCapsule(const FKSphylElem& Capsule, const FTransform& CompToWorld,
		TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms);

	/**
	 * Emits an FKConvexElem directly from its cooked VertexData / IndexData.
	 * Vertices are stored in element-local space; the element's own transform is baked into ElemToWorld by the caller.
	 * @param ConvexHull Convex element to emit.
	 * @param ElemToWorld Pre-composed element-to-world transform.
	 * @param OutMeshes Destination for the appended mesh.
	 * @param OutTransforms Destination for the appended transform.
	 */
	static void AppendChaosConvexHull(const FKConvexElem& ConvexHull, const FTransform& ElemToWorld,
		TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms);

	/**
	 * Tessellates an FKSphereElem as a UV sphere of SphereSegments x SphereRings resolution.
	 * The element's Center is folded into the emitted transform.
	 * @param Sphere Sphere element to emit.
	 * @param CompToWorld Owning component's component-to-world transform.
	 * @param OutMeshes Destination for the appended mesh.
	 * @param OutTransforms Destination for the appended transform.
	 */
	static void AppendChaosSphere(const FKSphereElem& Sphere, const FTransform& CompToWorld,
		TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms);

	/**
	 * Complex-as-simple, route 1: pulls LOD 0 positions and indices directly from the source
	 * UStaticMesh's render data. Preferred path for UStaticMeshComponent and its subclasses.
	 * @param StaticMesh Source static mesh to read from.
	 * @param ToWorld World-space transform to associate with the emitted mesh.
	 * @param OutMeshes Destination for the appended mesh.
	 * @param OutTransforms Destination for the appended transform.
	 * @return true when a mesh was appended; false when StaticMesh has no usable render data.
	 */
	static bool AppendComplexAsSimpleFromStaticMesh(const UStaticMesh* StaticMesh, const FTransform& ToWorld,
		TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms);

	/**
	 * Complex-as-simple, route 2: reads triangles from UBodySetup::TriMeshGeometries. Used as the fallback
	 * for non-StaticMesh primitives flagged as complex-as-simple (procedural meshes, destructibles, etc).
	 * @param Body Body setup whose Chaos tri meshes will be read.
	 * @param CompToWorld Owning component's component-to-world transform.
	 * @param OutMeshes Destination for the appended meshes (one per valid tri mesh).
	 * @param OutTransforms Destination for the appended transforms, parallel to OutMeshes.
	 * @note Caller must ensure the body setup's physics meshes have been built (UBodySetup::CreatePhysicsMeshes) before invoking.
	 */
	static void AppendComplexAsSimpleFromChaosTriMeshes(const UBodySetup* Body, const FTransform& CompToWorld,
		TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms);

	/**
	 * Class-name heuristic to skip landscape primitives without taking a hard Landscape-module dependency.
	 * @param Prim Primitive component to inspect.
	 * @return true when the component's class name begins with "Landscape".
	 */
	static bool IsLandscapePrimitive(const UPrimitiveComponent* Prim);
};

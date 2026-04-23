// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRawMesh.h"
#include "PhysicsEngine/BoxElem.h"
#include "PhysicsEngine/ConvexElem.h"
#include "PhysicsEngine/SphereElem.h"
#include "PhysicsEngine/SphylElem.h"

/**
 * Builds FNRawMesh instances from Unreal's collision and rendering primitives.
 *
 * Each factory method emits mesh-local vertices plus, where applicable, an accompanying element-to-world
 * transform so the caller can place every emitted mesh back in the correct frame. The class is stateless;
 * every method is static. Two distinct routes are exposed for complex-as-simple bodies — FromStaticMesh
 * (route 1, render-data) and FromChaosBodySetup (route 2, cooked Chaos tri meshes).
 */
class NEXUSCORE_API FNRawMeshFactory
{
public:
	
	/**
	 * Walks each supplied actor's registered UPrimitiveComponents and extracts their simple-collision
	 * representation as FNRawMesh entries, along with parallel world-space transforms.
	 * - Only actors whose world-space bounds are fully contained within at least one of the
	 *   supplied ContainingBounds are processed. Passing an empty array skips the containment test entirely.
	 * - Aggregate-geometry path covers FKConvexElem, FKBoxElem, FKSphereElem and FKSphylElem (capsule).
	 * - Complex-as-simple falls back to the source static-mesh render data (route 1) or Chaos TriMeshGeometries (route 2).
	 * - UInstancedStaticMeshComponent emits one FNRawMesh entry per instance.
	 * - Landscape-based primitives are skipped.
	 * @param Actors Candidate actors to process. Pre-filter at the call site (e.g. via FNActorUtils::GetWorldActors).
	 * @param ContainingBounds Actor-bounds filter; an actor is processed when its bounds are contained within any one of these. Skipped when empty.
	 * @param OutMeshes Each mesh in element-local space, appended to the array.
	 * @param OutTransforms Matching world-space transform per entry in OutMeshes.
	 * @note In editor builds, force-flushes any pending async static-mesh compilation (via FNDeveloperUtils::WaitForStaticMeshCompilation)
	 *       so actor bounds and BodySetups are fully populated before reading.
	 */
	static void FromActorsInBounds(const TArray<AActor*>& Actors, const TArray<FBoxSphereBounds>& ContainingBounds,
		TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms);
	
	/**
	 * Emits an FKBoxElem as an 8-vertex / 12-triangle FNRawMesh. The element's Center and Rotation are
	 * folded into the emitted transform; the mesh itself is box-half-extent-local.
	 * @param Box Box element to emit.
	 * @param CompToWorld Owning component's component-to-world transform.
	 * @param OutMesh Destination for the new mesh.
	 * @param OutTransform Destination for the new transform.
	 * @return true when a mesh and transform were written; false when the element could not be emitted.
	 */
	static bool FromChaosBox(const FKBoxElem& Box, const FTransform& CompToWorld, FNRawMesh& OutMesh, FTransform& OutTransform);

	/**
	 * Emits an FKConvexElem directly from its cooked VertexData / IndexData.
	 * Vertices are stored in element-local space; unlike the box/sphere/sphyl emitters, this method does not output a transform —
	 * the caller is responsible for combining FKConvexElem::GetTransform() with the owning component-to-world.
	 * @param ConvexHull Convex element to emit.
	 * @param OutMesh Destination for the new mesh.
	 * @return true when the hull was written; false when the element has no cooked vertex or index data.
	 */
	static bool FromChaosConvexHull(const FKConvexElem& ConvexHull, FNRawMesh& OutMesh);

	/**
	 * Tessellates an FKSphereElem as a UV sphere of SphereSegments x SphereRings resolution.
	 * The element's Center is folded into the emitted transform.
	 * @param Sphere Sphere element to emit.
	 * @param CompToWorld Owning component's component-to-world transform.
	 * @param OutMesh Destination for for the new mesh.
	 * @param OutTransform Destination for the new transform.
	 * @return true when a mesh and transform were written; false when the element could not be emitted.
	 */
	static bool FromChaosSphere(const FKSphereElem& Sphere, const FTransform& CompToWorld, FNRawMesh& OutMesh, FTransform& OutTransform);

	/**
	 * Tessellates an FKSphylElem as a cylindrical band plus two hemispherical caps along the element's local Z axis.
	 * The element's Center and Rotation are folded into the emitted transform.
	 * @param Sphyl Capsule (sphyl) element to emit.
	 * @param CompToWorld Owning component's component-to-world transform.
	 * @param OutMesh Destination for the new mesh.
	 * @param OutTransform Destination for the new transform.
	 * @return true when a mesh and transform were written; false when the element could not be emitted.
	 */
	static bool FromChaosSphyl(const FKSphylElem& Sphyl, const FTransform& CompToWorld, FNRawMesh& OutMesh, FTransform& OutTransform);

	/**
	 * Reads triangles directly from a Chaos triangle-mesh implicit object into an FNRawMesh.
	 * Vertices are stored in the implicit object's local space; the caller is responsible for any world placement.
	 * @param TriMesh Source Chaos triangle-mesh implicit object.
	 * @param OutMesh Destination for the new mesh.
	 * @return true when the tri mesh was written; false when TriMesh is null or empty.
	 * @note Intended as the inner helper for FromChaosBodySetup; most call sites should prefer that entry point.
	 */
	static bool FromChaosTriMeshes(const Chaos::FTriangleMeshImplicitObjectPtr& TriMesh, FNRawMesh& OutMesh);

	/**
	 * Complex-as-simple, route 2: reads triangles from UBodySetup::TriMeshGeometries. Used as the fallback
	 * for non-StaticMesh primitives flagged as complex-as-simple (procedural meshes, destructibles, etc).
	 * @param Body Body setup whose Chaos tri meshes will be read.
	 * @param ToWorld Transform applied to each emitted mesh so it lands in world space.
	 * @param OutMeshes Destination for the new meshes (one per valid tri mesh).
	 * @param OutTransforms Destination for the per-mesh transforms, parallel to OutMeshes.
	 * @return true when at least one mesh was appended; false when the body setup has no usable tri mesh data.
	 * @note Caller must ensure the body setup's physics meshes have been built (UBodySetup::CreatePhysicsMeshes) before invoking.
	 */
	static bool FromChaosBodySetup(const UBodySetup* Body, const FTransform& ToWorld, TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms);

	/**
	 * Complex-as-simple, route 1: pulls LOD 0 positions and indices directly from the source
	 * UStaticMesh's render data. Preferred path for UStaticMeshComponent and its subclasses.
	 * @param StaticMesh Source static mesh to read from.
	 * @param OutMesh Destination for the new mesh.
	 * @return true when a mesh was appended; false when StaticMesh has no usable render data.
	 */
	static bool FromStaticMesh(const UStaticMesh* StaticMesh, FNRawMesh& OutMesh);
	
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
};
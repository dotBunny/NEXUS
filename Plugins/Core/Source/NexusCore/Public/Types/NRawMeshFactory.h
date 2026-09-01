// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRawMesh.h"
#include "PhysicsEngine/BoxElem.h"
#include "PhysicsEngine/ConvexElem.h"
#include "PhysicsEngine/SphereElem.h"
#include "PhysicsEngine/SphylElem.h"

/**
 * Provenance of one mesh emitted by FNRawMeshFactory::FromActorsInBounds — which primitive produced it, and where in
 * that primitive's output it sat.
 *
 * Emitted only when a caller asks for it, and parallel to that method's OutMeshes / OutTransforms. It exists so a
 * consumer that persists emitted geometry can name each piece precisely enough to recognize it again in a later
 * gather: the component identifies the source, InstanceIndex separates the instances of an instanced primitive, and
 * ElementOrdinal separates the several meshes one body can emit — one per aggregate-geometry element, or per Chaos
 * tri mesh. Together the three are stable across gathers for as long as the source geometry is unchanged.
 * @note Holds a borrowed component pointer, valid only while the gather that produced it is on the stack. A consumer
 *       that outlives the gather must resolve it to something durable (a name, a hash) before the array is discarded.
 */
struct FNRawMeshSource
{
	/** Primitive that produced the mesh. Never null in an emitted record. */
	const UPrimitiveComponent* Component = nullptr;

	/** Instance that produced it, for an instanced primitive; INDEX_NONE for every other primitive. */
	int32 InstanceIndex = INDEX_NONE;

	/** Ordinal of this mesh within the output of that one component/instance, counted from zero. */
	int32 ElementOrdinal = 0;
};

/**
 * Builds FNRawMesh instances from Unreal's collision and rendering primitives.
 *
 * Each factory method emits mesh-local vertices plus, where applicable, an accompanying element-to-world
 * transform so the caller can place every emitted mesh back in the correct frame. The class is stateless;
 * every method is static. Two distinct routes are exposed for complex-as-simple bodies — FromStaticMesh
 * (route 1, render-data) and FromChaosBodySetup (route 2, cooked Chaos tri meshes).
 * @see <a href="https://nexus-framework.com/docs/core/types/types/raw-mesh-factory/">FNRawMeshFactory</a>
 */
class NEXUSCORE_API FNRawMeshFactory
{
public:

	/**
	 * Walks each supplied actor's registered UPrimitiveComponents and extracts their simple-collision
	 * representation as FNRawMesh entries, along with parallel world-space transforms.
	 * - Only actors whose world-space bounds intersect at least one of the
	 *   supplied ContainingBounds are processed. Passing an empty array skips the overlap test entirely.
	 * - Primitives whose collision is disabled are skipped. Collision on the source asset is not enough — a component
	 *   set to No Collision occupies no space in the world, and reading its body setup anyway would report geometry
	 *   that nothing can hit. This is per primitive rather than per actor, so an actor that collides through one
	 *   component does not drag in its non-colliding ones.
	 * - Aggregate-geometry path covers FKConvexElem, FKBoxElem, FKSphereElem and FKSphylElem (capsule).
	 * - Complex-as-simple falls back to the source static-mesh render data (route 1) or Chaos TriMeshGeometries (route 2).
	 * - A body that emits no simple geometry falls back to its complex tri mesh, so a mesh whose only collision is
	 *   the complex one — the default for a sculpted or imported asset — is not silently skipped.
	 * - UInstancedStaticMeshComponent emits one FNRawMesh entry per instance.
	 * - Landscape-based primitives are skipped; FromLandscape is the way to obtain their surface.
	 * - Primitives carrying any of ComponentIgnoreTags are skipped, so part of an actor can be left out without
	 *   leaving out the actor.
	 * @param Actors Candidate actors to process. Pre-filter at the call site (e.g. via FNActorUtils::GetWorldActors).
	 * @param ContainingBounds Actor-bounds filter; an actor is processed when its bounds overlap any one of these. Skipped when empty.
	 * @param OutMeshes Each mesh in element-local space, appended to the array.
	 * @param OutTransforms Matching world-space transform per entry in OutMeshes.
	 * @param OutSources Optional; when supplied, receives one FNRawMeshSource per emitted mesh, parallel to OutMeshes.
	 *        Callers that only consume the geometry should leave it null — the records cost an allocation per mesh and
	 *        are only of use to a consumer that needs to identify the same element in a later gather.
	 * @param ComponentIgnoreTags Component tags (UActorComponent::ComponentTags) that exclude a primitive. Empty by
	 *        default, which reads every primitive as before.
	 * @note Component-level rather than actor-level, because the actor is frequently not the author's unit of choice:
	 *       a generator writes many primitives onto one container actor, and excluding it would take the whole
	 *       generated result with it. Tested per primitive, so an actor can contribute some of its geometry and
	 *       withhold the rest.
	 * @note Does not narrow the ContainingBounds test, which is still made against the whole actor. An actor made
	 *       entirely of ignored primitives is still walked; it simply emits nothing.
	 * @note In editor builds, force-flushes any pending async static-mesh compilation (via FNDeveloperUtils::WaitForStaticMeshCompilation)
	 *       so actor bounds and BodySetups are fully populated before reading.
	 */
	static void FromActorsInBounds(const TArray<AActor*>& Actors, const TArray<FBoxSphereBounds>& ContainingBounds,
		TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms, TArray<FNRawMeshSource>* OutSources = nullptr,
		const TArray<FName>& ComponentIgnoreTags = TArray<FName>());

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

	/**
	 * Sample a landscape's surface into a triangulated grid mesh, by tracing down onto it.
	 *
	 * Landscape is the one terrain FromActorsInBounds cannot read: its collision is a Chaos heightfield behind no
	 * UBodySetup, so the factory skips landscape primitives outright and anything built from it sees a hole where the
	 * ground is. Sampling reconstructs a usable surface without taking a Landscape module dependency, and suits the
	 * shape, since a heightfield is single valued in Z and a downward trace per grid point therefore misses nothing.
	 * @param LandscapeActor Landscape to sample. Its own bounds set the sampled area unless SampleBounds narrows it.
	 * @param GridSize Spacing between samples, in world units. Values at or below zero sample nothing.
	 * @param OutMesh Destination, in world space — the caller pairs it with an identity transform.
	 * @param SampleBounds Optional region of interest, clipping the sampled area in XY. Pass an invalid box (the
	 *        default) to sample the whole landscape. Worth supplying whenever the caller is already gathering against
	 *        a region: a landscape spans the level, so the unclipped cost is set by the landscape's size rather than
	 *        by how much of it the caller cares about.
	 * @return true when at least one triangle was produced.
	 * @note SampleBounds clips in XY only — the traces still span the landscape's own vertical extent, because the
	 *       ground beneath a region of interest routinely sits below the box describing it and a trace clipped to that
	 *       box would report a hole where there is solid ground.
	 * @note Traces the live physics scene, so this is game-thread only and yields nothing where none is initialized.
	 *       That is why it is a separate entry point rather than a branch inside FromActorsInBounds, which carries no
	 *       such constraint.
	 * @remark **Single-valued surfaces only.** One downward trace per grid point keeps the first hit belonging to the
	 *         actor, so a surface folding back over itself — an overhang, a cave roof, the underside of a closed shape
	 *         — contributes only its topmost face. That is not a limitation for landscape, which is a heightfield and
	 *         cannot be authored any other way, and it is why this is named for landscape rather than for terrain.
	 *         Terrain that *can* be two-sided (a Mesh Partition sphere) has a UBodySetup and belongs in
	 *         FromActorsInBounds, which reads its cooked triangles whole and makes no assumption about topology.
	 * @note Approximate by construction: the surface is reproduced to within GridSize, and a sample that hits
	 *       nothing (a hole in the landscape) drops the quads around it rather than guessing at them.
	 */
	static bool FromLandscape(const AActor* LandscapeActor, double GridSize, FNRawMesh& OutMesh,
		const FBox& SampleBounds = FBox(ForceInit));

	/**
	 * Sample every landscape among a set of actors, as the landscape counterpart to FromActorsInBounds.
	 *
	 * The two are complements over the same actor list, and a caller wanting all of a level's geometry runs both:
	 * FromActorsInBounds covers everything a UBodySetup can describe, this covers the one terrain that has none.
	 * @param Actors Candidate actors; anything FNActorUtils::IsLandscapeActor rejects is skipped.
	 * @param ContainingBounds Regions of interest, narrowing the sampled area. Skipped when empty, as in FromActorsInBounds.
	 * @param GridSize Spacing between samples, in world units. Values at or below zero emit nothing.
	 * @param OutMeshes Each sampled surface, in world space, appended to the array.
	 * @param OutTransforms Matching transform per entry — always identity, since the samples are already placed.
	 * @note The sampled area is the *union* of ContainingBounds, not one pass per entry: regions routinely overlap, and
	 *       a pass each would emit the shared ground once per overlap for the caller to carry. The cost of the ground
	 *       between two distant regions is bounded by the landscape's own extent either way.
	 * @note Game-thread only, and yields nothing without an initialized physics scene — see FromLandscape.
	 */
	static void FromLandscapesInBounds(const TArray<AActor*>& Actors, const TArray<FBoxSphereBounds>& ContainingBounds,
		double GridSize, TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms);

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
	 * @param StaticMesh Mesh whose LOD0 render buffers are inspected.
	 * @return true when StaticMesh's LOD0 render buffers can be read on the CPU. Always true in editor (CPU copies
	 *         are retained regardless of the asset flag); in cooked builds, true only when the mesh opted into CPU
	 *         access — otherwise the vertex/index CPU data is freed after GPU upload and FromStaticMesh would
	 *         dereference null buffer data.
	 * @note Guards every render-data read in FromActorsInBounds; route 2 (cooked Chaos tri mesh) is the fallback.
	 */
	static bool IsStaticMeshCPUReadable(const UStaticMesh* StaticMesh);
};
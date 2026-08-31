// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class AActor;
class UPrimitiveComponent;
struct FNRawMeshSource;

namespace NEXUS::WorldAssembly::CollisionKey
{
	/** Reciprocal of the position precision a hashed transform keeps, in world units (0.01cm). */
	constexpr double LocationQuantum = 100.0;

	/** Reciprocal of the precision a hashed transform keeps for each quaternion and scale component. */
	constexpr double RotationQuantum = 10000.0;
	constexpr double ScaleQuantum = 10000.0;
}

/**
 * Durable identity for one piece of world collision geometry, plus the hash primitives the collision-cache system
 * shares.
 *
 * A key names a single mesh emitted by FNRawMeshFactory::FromActorsInBounds precisely enough that a later gather —
 * in another editor session, or in a packaged build — produces the same value for the same geometry. That is what
 * lets the level's collision pool store each element once and lets an organ reference its elements by key rather
 * than by array index: a key that no longer resolves is a detectable miss, where a stale index would silently name
 * whatever geometry now sits at that position.
 *
 * Identity is (level package, actor name, component name, instance index, element ordinal). All five are saved
 * state, so all five survive a round trip through the map on disk.
 * @note Deliberately excludes the geometry's transform and body. Those belong to the *fingerprint*, which decides
 *       whether a cache is still valid; a key answers only "which element is this", so that moving an actor updates
 *       its pool entry rather than orphaning it and leaking a new one.
 * @see FNWorldCollisionFingerprint
 */
class NEXUSWORLDASSEMBLY_API FNWorldCollisionSourceKey
{
public:
	/**
	 * @return The key for a mesh the raw-mesh factory just emitted, or 0 when the record names no component.
	 * @param Source Provenance record parallel to the factory's emitted mesh array.
	 */
	static uint64 FromSource(const FNRawMeshSource& Source);

	/**
	 * @return The key for one element of one primitive, or 0 when Component is null.
	 * @param Component Primitive that produced the geometry.
	 * @param InstanceIndex Instance that produced it for an instanced primitive; INDEX_NONE otherwise.
	 * @param ElementOrdinal Ordinal of the mesh within that component/instance's output.
	 */
	static uint64 FromComponent(const UPrimitiveComponent* Component, int32 InstanceIndex, int32 ElementOrdinal);

	/**
	 * @return The identity of the actor a key is rooted in — its level package plus its own name — or 0 when null.
	 * @param Actor Actor to identify.
	 * @note Uses the *level's* package rather than the actor's own, so an actor saved into an external package by
	 *       World Partition keys the same as one saved inline. Actor names are unique within a level and are saved,
	 *       which is what makes the pair stable; an actor spawned at runtime gets a generated name and so correctly
	 *       fails to match anything baked.
	 */
	static uint64 FromActor(const AActor* Actor);

	/** @return Value passed through a splitmix64 finalizer, so that near-identical inputs land far apart. */
	static uint64 Mix(uint64 Value);

	/** @return Seed folded with Value, order-dependently. Use for sequential composition, never for set membership. */
	static uint64 Combine(uint64 Seed, uint64 Value);

	/**
	 * @return A stable hash of Name's text.
	 * @note Hashed by text rather than by FName comparison index, which is assigned per session and would differ
	 *       between the editor that baked a cache and the game that reads it.
	 */
	static uint64 HashName(const FName& Name);

	/** @return A stable hash of Value, computed over its UTF-8 encoding so the result is platform-independent. */
	static uint64 HashString(const FString& Value);

	/** @return A stable hash of Guid's four components. */
	static uint64 HashGuid(const FGuid& Guid);

	/**
	 * @return A hash of Transform, quantized so that floating-point noise does not read as a change.
	 * @note Quantization has a boundary: a component sitting exactly on a quantum edge can hash either way between
	 *       runs. That costs a spurious cache miss and a fresh gather, never a wrongly-accepted cache.
	 */
	static uint64 HashTransform(const FTransform& Transform);

	/** @return A hash of Vector quantized at LocationQuantum. */
	static uint64 HashVector(const FVector& Vector);
};

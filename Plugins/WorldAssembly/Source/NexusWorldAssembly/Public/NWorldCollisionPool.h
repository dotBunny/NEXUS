// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Types/NRawMesh.h"
#include "NWorldCollisionPool.generated.h"

/**
 * A level's baked world-collision geometry, stored once per element and addressed by key.
 *
 * The pool is the single home for every hull an assembly in this level might test against. Organs do not store
 * geometry of their own — each holds a list of keys into this pool (see FNOrganCollisionCache) — which is what keeps
 * two unbounded organs, or any pair whose volumes overlap, from each carrying a private copy of the same geometry.
 *
 * Meshes are stored in their finished form: world-space, convex, and validated, exactly as FNProcessVirtualWorldTask
 * would have left them. Reading the pool therefore skips both the gather and the bake.
 *
 * Landscape lives in its own section rather than in the keyed one. It is sampled unbounded by design — an assembly
 * reads the ground well outside any organ volume — so no organ references it and no key would mean anything; it is
 * added to every assembly that asks for landscape, and carries its own fingerprint.
 * @note Not thread-safe. The lookup index is built lazily on first query and mutated by Add / RemoveUnreferenced.
 * @see FNOrganCollisionCache
 * @see FNWorldCollisionSourceKey
 */
USTRUCT()
struct NEXUSWORLDASSEMBLY_API FNWorldCollisionPool
{
	GENERATED_BODY()

	/** Serialized-format version, written first so an older pool can be recognized and discarded rather than misread. */
	static constexpr uint8 CurrentVersion = 1;

	/** Keys of the pooled elements, parallel to Meshes. */
	TArray<uint64> Keys;

	/** Baked world-space hulls, parallel to Keys. */
	TArray<FNRawMesh> Meshes;

	/** Sampled landscape surface for this level; referenced by no key. Empty when landscape is not being captured. */
	TArray<FNRawMesh> LandscapeMeshes;

	/** Fingerprint of the landscape state LandscapeMeshes was sampled from; 0 when never sampled. */
	uint64 LandscapeFingerprint = 0;

	/**
	 * The world origin the vertices were baked against.
	 *
	 * Hulls are stored in world space, so a world whose origin has been rebased since the bake would place every one
	 * of them off by that shift — an error that no fingerprint would catch, because none of the actors changed. A
	 * consumer compares this against the live world's origin and refuses the cache when they differ.
	 * @note Zero for an ordinary level, which is every level that does not enable origin rebasing.
	 */
	FIntVector BakeOriginLocation = FIntVector::ZeroValue;

	/** @return The number of keyed elements held. */
	int32 Num() const { return Keys.Num(); }

	/** @return true when the pool holds neither keyed elements nor landscape. */
	bool IsEmpty() const { return Keys.IsEmpty() && LandscapeMeshes.IsEmpty(); }

	/** Drop every element, the landscape section, and the lookup index. */
	void Reset();

	/** @return true when Key names an element in the pool. */
	bool Contains(uint64 Key) const;

	/**
	 * Add or replace one element.
	 * @param Key Identity of the element; a key already present is overwritten in place.
	 * @param Mesh Baked world-space hull, moved into the pool.
	 */
	void Add(uint64 Key, FNRawMesh&& Mesh);

	/**
	 * Append the hulls named by Keys to OutMeshes.
	 * @param InKeys Keys to resolve.
	 * @param OutMeshes Destination, appended to.
	 * @return true when every key resolved; false when at least one was missing, in which case the caller must treat
	 *         the referencing cache as stale and gather afresh.
	 * @remark Resolving by key rather than by index is what makes a stale reference detectable. An index list against
	 *         a pool that has since changed would silently name whatever geometry now occupies that slot.
	 */
	bool Resolve(const TArray<uint64>& InKeys, TArray<FNRawMesh>& OutMeshes) const;

	/**
	 * Drop every keyed element not named in Referenced.
	 * @param Referenced Union of the key sets of every organ in the level.
	 * @return The number of elements removed.
	 * @remark This is the whole of the pool's lifetime management. Because a bake recomputes the referencing key sets
	 *         before calling this, an element outliving its last referent is collected without any need for refcounts.
	 */
	int32 RemoveUnreferenced(const TSet<uint64>& Referenced);

	/** Versioned read/write of both sections. */
	bool Serialize(FArchive& Ar);

	/**
	 * @return true when Other holds the same baked geometry as this pool.
	 * @param Other Pool to compare against; null is never identical.
	 * @param PortFlags Unused; this comparison has no text/binary distinction.
	 * @remark Required, not an optimization — see FNOrganCollisionCache::Identical for the same reasoning. None of
	 *         this struct's state is reflected, so without this UE decides every pool equals the empty default and
	 *         never writes it, and a baked level loads back with nothing in it.
	 * @note Ordered cheapest first. Against the empty default every real pool differs at the key count, so the
	 *       per-vertex comparison below is only reached by two pools that already agree on everything else.
	 */
	bool Identical(const FNWorldCollisionPool* Other, uint32 PortFlags) const;

private:
	/** Build the key-to-index map if it is not current. */
	void EnsureIndex() const;

	/**
	 * Read or write one mesh's persisted state.
	 *
	 * Written field by field rather than through the struct's reflection so the payload carries no property tags — a
	 * pool holds one record per collision element in the level, and tagged serialization would roughly double it. The
	 * transient face-plane cache is deliberately absent; rebuilding it is cheaper than storing it.
	 * @note A member rather than a free helper because FNRawMesh's validation flags are private and this type is on
	 *       its friend list. Persisting those flags is the point: they were settled during the bake, and a loaded mesh
	 *       leaves its dirty flag clear, so the bake's answer stands instead of being recomputed on first use.
	 */
	static void SerializeMesh(FArchive& Ar, FNRawMesh& Mesh);

	/** Read or write a whole mesh array, length-prefixed. */
	static void SerializeMeshArray(FArchive& Ar, TArray<FNRawMesh>& Meshes);

	/** Lazily built lookup over Keys; never serialized, rebuilt on demand after a load or a mutation. */
	mutable TMap<uint64, int32> KeyIndex;

	/** True while KeyIndex matches Keys. */
	mutable bool bKeyIndexValid = false;
};

template<>
struct TStructOpsTypeTraits<FNWorldCollisionPool> : TStructOpsTypeTraitsBase2<FNWorldCollisionPool>
{
	enum
	{
		WithSerializer = true,
		// Without this the serializer above never runs; see FNWorldCollisionPool::Identical.
		WithIdentical = true
	};
};

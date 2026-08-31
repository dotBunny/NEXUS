// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NOrganCollisionCache.generated.h"

/**
 * One organ's record of the world collision it was baked against.
 *
 * Holds no geometry. The hulls live once in the level's FNWorldCollisionPool, and an organ names the ones it needs
 * by key — so an organ whose volume overlaps another's, or two unbounded organs that both cover the level, cost one
 * key list each rather than a private copy of the geometry.
 *
 * The fingerprint is the whole of the validity test: it summarizes the region, the settings, and every actor the
 * gather would have read, so a cache whose fingerprint still matches the live world describes that world exactly.
 * A mismatch is not an error — it means the level changed since the bake, and the assembly gathers afresh.
 * @see FNWorldCollisionPool
 * @see FNWorldCollisionFingerprint
 */
USTRUCT()
struct NEXUSWORLDASSEMBLY_API FNOrganCollisionCache
{
	GENERATED_BODY()

	/** Serialized-format version; an unrecognized one is discarded rather than misread. */
	static constexpr uint8 CurrentVersion = 1;

	/** Fingerprint of the world this organ was baked against; 0 when never baked. */
	uint64 Fingerprint = 0;

	/** Sorted, duplicate-free keys into the level's pool. */
	TArray<uint64> SourceKeys;

	/** When the bake ran, for the details panel and for diagnosing a cache that keeps missing. */
	FDateTime BakeTime;

	/** @return true when a bake has been recorded, regardless of whether it is still current. */
	bool HasData() const { return Fingerprint != 0; }

	/**
	 * @return true when this cache still describes the world.
	 * @param CurrentFingerprint Fingerprint computed from the live world for this organ's bounds and settings.
	 */
	bool IsValidFor(const uint64 CurrentFingerprint) const
	{
		return Fingerprint != 0 && Fingerprint == CurrentFingerprint;
	}

	/** Clear the cache back to never-baked. */
	void Reset()
	{
		Fingerprint = 0;
		SourceKeys.Reset();
		BakeTime = FDateTime();
	}

	/**
	 * Record the result of a bake.
	 * @param InFingerprint Fingerprint the bake ran against.
	 * @param InKeys Keys the bake produced; sorted and de-duplicated into the cache.
	 * @note Sorted so the stored order does not depend on the order the world was walked in, which keeps a re-bake
	 *       of unchanged geometry byte-identical and out of the next changelist.
	 */
	void SetBakeResult(uint64 InFingerprint, TArray<uint64>&& InKeys);

	/** Versioned read/write. */
	bool Serialize(FArchive& Ar);

	/**
	 * @return true when Other holds the same bake as this one.
	 * @param Other Cache to compare against; null is never identical.
	 * @param PortFlags Unused; this comparison has no text/binary distinction.
	 * @remark Required, not an optimization. None of this struct's state is reflected — a `uint64` cannot be a
	 *         UPROPERTY, which is why it carries a hand-written serializer — so without this UE compares it by walking
	 *         its (empty) property list, concludes every cache equals the default, and skips writing the property
	 *         entirely. The serializer below is then never even reached, and a baked organ loads back unbaked.
	 */
	bool Identical(const FNOrganCollisionCache* Other, uint32 PortFlags) const
	{
		return Other != nullptr
			&& Fingerprint == Other->Fingerprint
			&& BakeTime == Other->BakeTime
			&& SourceKeys == Other->SourceKeys;
	}
};

template<>
struct TStructOpsTypeTraits<FNOrganCollisionCache> : TStructOpsTypeTraitsBase2<FNOrganCollisionCache>
{
	enum
	{
		WithSerializer = true,
		// Without this the serializer above never runs; see FNOrganCollisionCache::Identical.
		WithIdentical = true
	};
};

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
// FOrganResult holds meshes by value, so the type has to be complete here rather than forward-declared.
#include "Types/NRawMesh.h"

class AActor;
class UNOrganComponent;
class UWorld;
struct FNWorldAssemblyWorldCollisionSettings;
struct FNWorldCollisionPool;

/**
 * Produces the baked geometry a collision cache holds.
 *
 * The baker is the one place that knows how to turn a live world into pooled hulls. It performs the same gather the
 * assembly pipeline performs — FNCreateVirtualWorldTask's actor filter and raw-mesh extraction, then the transform
 * bake, convex conversion, and face-plane warm that FNProcessVirtualWorldTask performs — and stores the finished
 * result, so that reading a cache later skips both stages rather than one.
 *
 * Baking is an author-time act driven from the ed mode or from a level save, but the code lives in the runtime
 * module deliberately: the gather half is also what a runtime cache miss falls back to, and having one definition of
 * "what geometry does this organ see" is what keeps a cache hit and a cache miss from producing different worlds.
 * @note Game-thread only. Walks live actors, traces the physics scene for landscape, and flushes async static-mesh
 *       compilation in editor builds.
 * @see FNWorldCollisionPool
 * @see FNOrganCollisionCache
 */
class NEXUSWORLDASSEMBLY_API FNWorldCollisionBaker
{
public:
	/** What a level's baked pool could answer for a set of organs. */
	struct FPooledCollision
	{
		/** Baked world-space hulls resolved from the pool, ready to use as-is. */
		TArray<FNRawMesh> Meshes;

		/** Keys satisfied here, so a supplementary gather can skip re-emitting the same elements. */
		TSet<uint64> ResolvedKeys;

		/** Organs the pool could not answer for; each still needs gathering from the live world. */
		TArray<UNOrganComponent*> MissedOrgans;

		/** True when a cache actor exists, holds something, and was baked against this world's origin. */
		bool bPoolUsable = false;

		/** True when landscape was asked for and its section was current; false means it must be sampled live. */
		bool bLandscapeResolved = false;

		/** @return true when the pool answered for every organ asked about. */
		bool IsComplete() const { return bPoolUsable && MissedOrgans.IsEmpty(); }
	};

	/**
	 * Resolve whatever the level's baked pool can answer for a set of organs.
	 *
	 * The single implementation of "read the cache", shared by the assembly's capture phase and by the editor's
	 * collision preview. Both need the same three judgements — is there a pool, was it baked against this world, and
	 * is each organ's fingerprint still current — and a second copy of them is how the two would drift apart.
	 * @param World World to read; a null world resolves nothing.
	 * @param Settings Collision-capture settings, fingerprinted as part of validity.
	 * @param Organs Organs to consult. An empty list resolves nothing, since the pool is addressed through them.
	 * @param bValidate When false, an organ's stored cache is taken at its word rather than fingerprinted.
	 * @param bIncludeLandscape When true, the level-scoped landscape section is appended if its fingerprint is current.
	 * @return What could be resolved, and what was left over.
	 * @note Game-thread only when bValidate is set — fingerprinting walks live actors.
	 */
	static FPooledCollision ResolvePooled(const UWorld* World, const FNWorldAssemblyWorldCollisionSettings& Settings,
		const TArray<UNOrganComponent*>& Organs, bool bValidate, bool bIncludeLandscape);

	/** @return Every organ component in World's non-instanced levels. */
	static TArray<UNOrganComponent*> GetWorldOrgans(const UWorld* World);

	/** What one organ's bake produced. */
	struct FOrganResult
	{
		/** Fingerprint of the world the bake ran against. */
		uint64 Fingerprint = 0;

		/** Keys of the elements the organ needs, parallel to Meshes. */
		TArray<uint64> Keys;

		/** Baked world-space hulls, parallel to Keys. */
		TArray<FNRawMesh> Meshes;
	};

	/**
	 * Bake one gathered element into the form the assembly actually tests against: world-space, convex, face-planes
	 * warmed.
	 *
	 * The single definition of that transformation, and deliberately public. It began as a copy of the loop in
	 * FNProcessVirtualWorldTask, and while there were two copies the editor's collision visualizer quietly grew a
	 * third behaviour — it never convexified at all, so it drew concave geometry that no part of an assembly honours.
	 * Everything that needs to say "what will assembly collide with here" routes through this instead.
	 * @param Mesh Element to bake, in element-local space; replaced in place with its baked form.
	 * @param Transform World transform for the element.
	 * @note Thread-safe. Pure FNRawMesh math with no UObject access, so it runs on the assembly's worker threads as
	 *       readily as on the game thread.
	 */
	static void BakeElement(FNRawMesh& Mesh, const FTransform& Transform);

	/**
	 * Gather and bake the world collision one organ sees.
	 * @param World World to read.
	 * @param Bounds Containment bounds for this organ; empty means the whole level, as everywhere else.
	 * @param Settings Collision-capture settings governing which actors qualify.
	 * @param OutResult Receives the fingerprint, keys, and baked hulls.
	 * @return true when the bake ran; false when World is null.
	 * @note Landscape is not included. It is unbounded by design and belongs to the level rather than to any organ —
	 *       see BakeLandscape.
	 */
	static bool BakeOrgan(const UWorld* World, const TArray<FBoxSphereBounds>& Bounds,
		const FNWorldAssemblyWorldCollisionSettings& Settings, FOrganResult& OutResult);

	/**
	 * Sample and bake the level's landscape surface into a pool's landscape section.
	 * @param World World to read.
	 * @param Settings Collision-capture settings; a run with landscape capture off clears the section instead.
	 * @param Pool Pool whose landscape section and fingerprint are replaced.
	 * @return true when the section changed, so a caller can tell whether the level needs saving.
	 * @note The expensive one: a landscape contributes one downward physics trace per sample, and the sampled area is
	 *       the whole landscape rather than any organ's volume. This is the cost the level-scoped section exists to
	 *       stop every organ from paying separately.
	 */
	static bool BakeLandscape(const UWorld* World, const FNWorldAssemblyWorldCollisionSettings& Settings,
		FNWorldCollisionPool& Pool);

#if WITH_EDITOR
	/**
	 * Broadcast after a bake has finished writing to a world's pool.
	 *
	 * A bake is the only thing that changes what the pool holds, and everything derived from it — the editor's merged
	 * preview mesh and its BVH among them — is memoized against that. Announcing the bake is what keeps those in step
	 * without each new bake site having to remember to go and invalidate them by hand.
	 */
	DECLARE_MULTICAST_DELEGATE_OneParam(FNOnWorldCollisionBaked, const UWorld* /* World */);
	static FNOnWorldCollisionBaked OnBaked;

	/** What a whole-level or multi-organ bake did. */
	struct FBakeResult
	{
		/** How many organs had their cache rewritten. Organs still matching their fingerprint are not counted. */
		int32 OrgansBaked = 0;

		/** True when something was written, and the level therefore needs saving. */
		bool bChanged = false;

		/**
		 * True when the user cancelled part-way.
		 *
		 * The organs already baked keep their new caches and the rest keep their old ones, so the level is left
		 * consistent — every organ's stored fingerprint still describes whatever it was last baked against. What a
		 * caller must not do is conclude the level has been fully examined: the save-time pass relies on that to
		 * decide it can skip the next save.
		 */
		bool bCancelled = false;
	};

	/**
	 * Bake every organ in World, write the results into the level's pool, and drop whatever nothing references.
	 * @param World World to bake.
	 * @param Settings Collision-capture settings governing which actors qualify.
	 * @param bForce When true, re-bake even organs whose fingerprint still matches.
	 * @return What the bake did.
	 * @remark Organs whose fingerprint still matches are skipped, which is what makes a save-time bake cheap on a
	 *         level nobody has touched: the cost is one fingerprint pass per organ, not a re-gather.
	 */
	static FBakeResult BakeWorld(UWorld* World, const FNWorldAssemblyWorldCollisionSettings& Settings, bool bForce);

	/**
	 * Bake a named subset of organs rather than the whole level.
	 * @param World World to bake.
	 * @param Organs Organs to consider; others keep whatever cache they already hold.
	 * @param Settings Collision-capture settings governing which actors qualify.
	 * @param bForce When true, re-bake even organs whose fingerprint still matches.
	 * @return What the bake did.
	 * @note Reports progress through a slow task and can be cancelled from it. The work is game-thread and
	 *       synchronous — a gather flushes pending static-mesh compilation and landscape sampling traces the physics
	 *       scene once per sample — so on a large level this is seconds to minutes, not milliseconds.
	 */
	static FBakeResult BakeOrgans(UWorld* World, const TArray<UNOrganComponent*>& Organs,
		const FNWorldAssemblyWorldCollisionSettings& Settings, bool bForce);
#endif // WITH_EDITOR

	/** What region of the world an organ contributes to a gather. */
	enum class EOrganBoundsKind : uint8
	{
		/** The organ's volume bounds; OutBounds holds them. */
		Bounded,
		/** The whole level. OutBounds is empty, which every gather reads as "no filter". */
		Unbounded,
		/** Nothing at all. OutBounds is empty, and the organ must be left out of the gather entirely. */
		None
	};

	/**
	 * @return What region Organ contributes, and through OutBounds the bounds themselves when it is Bounded.
	 * @param Organ Organ to measure.
	 * @param OutBounds Receives the organ's volume bounds for a Bounded organ; emptied otherwise.
	 * @remark Three answers rather than two because an empty bounds array already means "the whole level" everywhere
	 *         else, and two of these cases produce one. Mirrors FNAssemblyOperationContext::LockAndPreprocess exactly:
	 *         any unbound organ makes the gather read the whole level, while a bounded organ with no volume to bound
	 *         it is skipped and contributes nothing. Reading that second case as "whole level" would make a cached run
	 *         see more geometry than a fresh one — the one divergence this whole system must not have.
	 */
	static EOrganBoundsKind GetOrganBounds(const UNOrganComponent* Organ, TArray<FBoxSphereBounds>& OutBounds);
};

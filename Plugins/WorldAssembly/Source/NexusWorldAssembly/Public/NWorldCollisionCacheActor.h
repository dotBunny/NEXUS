// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NWorldCollisionPool.h"
#include "GameFramework/Info.h"
#include "NWorldCollisionCacheActor.generated.h"

/**
 * Level-scoped holder for the baked world-collision pool.
 *
 * One per level, created by the bake and found by scanning for it. It exists as an actor rather than as data hung
 * off AWorldSettings so that World Partition gives the pool its own external package: every bake would otherwise
 * check out the persistent level, which is the single most contended file in a team's stream.
 *
 * Nothing places this by hand — the ed mode's cache action and the save-time bake create it on demand, and it holds
 * no authored state, so deleting it costs a rebake and nothing else.
 * @see FNWorldCollisionPool
 */
UCLASS(NotPlaceable, NotBlueprintable, ClassGroup = "NEXUS", DisplayName = "NEXUS | World Collision Cache",
	HideCategories = (Rendering, Replication, Collision, Input, Actor, LOD, Cooking, HLOD, Physics, Networking))
class NEXUSWORLDASSEMBLY_API ANWorldCollisionCacheActor : public AInfo
{
	GENERATED_BODY()

public:
	explicit ANWorldCollisionCacheActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** The level's baked collision geometry. */
	UPROPERTY()
	FNWorldCollisionPool Pool;

	/**
	 * @return The cache actor in World, or nullptr when the level has never been baked.
	 * @param World World to search; null returns nullptr.
	 * @note Skips instanced levels, so a level opened as a level instance reads its own cache rather than its host's.
	 */
	static ANWorldCollisionCacheActor* Find(const UWorld* World);

#if WITH_EDITOR
	/**
	 * @return The cache actor in World, spawning one into the persistent level if there is none.
	 * @param World World to search or spawn into; null returns nullptr.
	 * @note Editor-only. A bake is the only thing that creates this, and a bake only ever happens at author time.
	 */
	static ANWorldCollisionCacheActor* FindOrCreate(UWorld* World);
#endif // WITH_EDITOR
};

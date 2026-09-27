// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Macros/NSubsystemMacros.h"
#include "Misc/App.h"
#include "NUISettings.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/ObjectKey.h"
#include "NTextRenderWarmupSubsystem.generated.h"

class UFont;
class ULevel;
class ULevelStreaming;
class UMaterialInterface;
class UTextRenderComponent;

/**
 * Creates every text render's font materials on the game thread, before the level holding it registers.
 *
 * A text render's scene proxy asks the engine's text MID cache for one material instance per font page, and the cache
 * creates them the first time a (material, font) pair is asked for. A level being loaded or streamed in adds its
 * primitives from a ParallelFor (FRegisterComponentContext::Process), so the first text render of a pair in a level
 * creates its MIDs on a worker: a Debug build asserts in the material's profile getters (their game thread checks do
 * not allow the parallel game thread, and Substrate calls them), and every other build creates UObjects off the game
 * thread without saying so.
 *
 * For each pair a loaded text render uses, this registers one empty, ownerless text render on the game thread first,
 * and keeps it registered: its proxy puts the pair in the cache and holds it there (the cache purges only pairs nothing
 * holds), so the level's own text renders find their MIDs made.
 *
 * The pairs come from the engine's list of live objects per class, never a walk of a level's actors, so a look costs
 * as much as there are text renders loaded. One is taken at PostInitialize, which InitWorld runs ahead of LoadMap
 * registering the persistent level, and one whenever a streamed level or World Partition cell begins becoming visible,
 * which AddToWorld broadcasts after the level has loaded and before it registers. A text render spawned at runtime
 * registers on the game thread already.
 *
 * Created for Game and PIE worlds on a machine that renders, unless UNUISettings::bWarmTextRenderMaterials is off.
 * @see <a href="https://nexus-framework.com/docs/ui/types/text-render-warmup-subsystem/">UNTextRenderWarmupSubsystem</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Text Render Warmup Subsystem")
class NEXUSUI_API UNTextRenderWarmupSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	// Nothing makes a scene proxy where nothing renders: a dedicated server, or a -nullrhi run.
	N_WORLD_SUBSYSTEM_GAME_ONLY(UNTextRenderWarmupSubsystem,
		UNUISettings::Get()->bWarmTextRenderMaterials && FApp::CanEverRender())

public:
	//~USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//End USubsystem

	//~UWorldSubsystem
	virtual void PostInitialize() override;
	//End UWorldSubsystem

	/**
	 * Warms every (material, font) pair a loaded text render uses.
	 * @note The subsystem calls this itself at world initialization and whenever a level begins becoming visible; call
	 *       it after loading text renders any other way, before a parallel registration reaches them.
	 */
	void WarmLoadedTextRenders();

	/**
	 * Whether a (material, font) pair is warm in this world.
	 * @param Material The text render's TextMaterial; null is the engine's default material, as it is to the proxy.
	 * @param Font The text render's font.
	 * @return true once a text render drawing with the pair is registered by this subsystem.
	 */
	bool IsWarm(const UMaterialInterface* Material, const UFont* Font) const;

	/** How many (material, font) pairs are warm in this world. */
	int32 GetWarmPairCount() const { return WarmPairs.Num(); }

private:
	void OnLevelBeginMakingVisible(UWorld* InWorld, const ULevelStreaming* InStreamingLevel, ULevel* InLevel);
	void OnWorldCleanup(UWorld* InWorld, bool bSessionEnded, bool bCleanupResources);

	/** Registers an empty text render drawing with the pair, unless the pair is warm already. */
	void WarmPair(UMaterialInterface* Material, UFont* Font);

	/**
	 * Destroys every warm text render.
	 * @note Must run before the world is cleaned up: an ownerless component still registered then is reported as
	 *       leaked.
	 */
	void ReleaseAll();

	/** One registered text render per warm pair; its scene proxy is what holds the pair in the cache. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextRenderComponent>> WarmComponents;

	TSet<TPair<TObjectKey<UMaterialInterface>, TObjectKey<UFont>>> WarmPairs;

	FDelegateHandle LevelBeginMakingVisibleHandle;
	FDelegateHandle WorldCleanupHandle;
};

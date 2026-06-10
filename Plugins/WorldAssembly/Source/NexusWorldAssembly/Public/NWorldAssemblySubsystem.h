// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/INAssemblyOperationOwner.h"
#include "Macros/NSubsystemMacros.h"
#include "Assembly/NAssemblyOperationSettings.h"
#include "NWorldAssemblyRelay.h"
#include "Types/NSimpleDelegates.h"
#include "NWorldAssemblySubsystem.generated.h"

class UNOrganComponent;
class ANWorldAssemblyRelay;
class UNAssemblyOperation;
class FNAssemblyTaskGraph;
class ANCellProxy;
class ANCellActor;
class UNWorldAssemblyContext;
class APlayerController;
class AController;
class AGameModeBase;


/**
 * Runtime world subsystem that drives World Assembly generation during gameplay.
 *
 * Game-only; also implements INAssemblyOperationOwner so it can host operations created via Generate().
 * Spawns one ANWorldAssemblyRelay per connected player controller to carry per-player generation state.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | World Assembly Subsystem")
class NEXUSWORLDASSEMBLY_API UNWorldAssemblySubsystem : public UTickableWorldSubsystem, public INAssemblyOperationOwner
{
	GENERATED_BODY()
	N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY(UNWorldAssemblySubsystem, true)

public:
	/**
	 * Kick off a new generation pass with the supplied per-operation settings.
	 * @param Settings Operation-level settings (seed, level-instance behavior); taken by reference so the caller can reuse the struct.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Generate", Category = "NEXUS|WorldAssembly")
	void Generate(UPARAM(ref) FNAssemblyOperationSettings& Settings);

	/**
	 * Tear down every assembled object owned by the subsystem and return it to an empty state.
	 *
	 * Cancels any in-flight operations, destroys every ANCellProxy in the world along with its streamed level instance,
	 * destroys any actors previously enrolled via RegisterActorForCleanup, then empties the tracked-actor list and broadcasts OnCleared.
	 * In editor builds the global selection is cleared first so the typed-element registry does not assert on a stale handle
	 * after sub-level actors are torn down.
	 * @note Does not destroy player relays — those are tied to player controller lifetime, not generation lifetime.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Clear", Category = "NEXUS|WorldAssembly")
	void Clear();
	
	/** @return Relay associated with the local player, or nullptr if it has not yet been spawned. */
	UFUNCTION(BlueprintCallable, DisplayName="Get Local Relay", Category = "NEXUS|WorldAssembly")
	ANWorldAssemblyRelay* GetLocalRelay() const { return LocalRelay; }

	/**
	 * @param bWaitOnStreaming When true, also report not-ready while any level streaming is still in flight (see FNWorldUtils::IsStreaming). Pass false to ignore streaming and gate purely on operation/relay state.
	 * @return true when the local WorldAssembly view is settled relative to the server.
	 * @remark Server path: no operations are currently in flight. Client path: LocalRelay has replicated, the nearby-cell payload has been received, and no operations the client has been notified of are pending.
	 * @note Does not gate Generate() — that can be called at any time regardless of this value.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Is Ready?", Category = "NEXUS|WorldAssembly")
	bool IsReady(bool bWaitOnStreaming = true);
	
	UFUNCTION(BlueprintCallable, DisplayName="Get Remaining Status", Category = "NEXUS|WorldAssembly", meta=(ToolTip="Gets the remaining Cell Level Instances to sync (Remaining/Total) on clients. Zero if server."))
	FIntVector2 GetRemainingStatus();
	
	/**
	 * Track an externally-owned actor so it will be destroyed by the next Clear() pass.
	 *
	 * Stored as a weak reference, so the actor is free to be destroyed by other systems first without leaving a dangling entry.
	 * Safe to call repeatedly with the same actor — duplicates are ignored.
	 * @param Actor Actor to enroll in cleanup. Null is tolerated but ignored.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Register Actor For Cleanup", Category = "NEXUS|WorldAssembly")
	void RegisterActorForCleanup(AActor* Actor);

	/**
	 * Stop tracking an actor for Clear()-driven destruction.
	 *
	 * Call when the actor's lifetime is taken over elsewhere, or when it has already been destroyed and the slot should be reclaimed early.
	 * @param Actor Actor previously passed to RegisterActorForCleanup. A no-op if the actor was never registered.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Unregister Actor For Cleanup", Category = "NEXUS|WorldAssembly")
	void UnregisterActorForCleanup(AActor* Actor);

	//~UTickableWorldSubsystem
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
	N_TICKABLE_WORLD_SUBSYSTEM_GET_TICKABLE_TICK_TYPE(ETickableTickType::Conditional)
	//End UTickableWorldSubsystem

	//~INAssemblyOperationOwner
	virtual void StartOperation(UNAssemblyOperation* Operation) override;
	virtual void OnOperationFinished(UNAssemblyOperation* Operation, TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContext) override;
	virtual void OnOperationDestroyed(UNAssemblyOperation* Operation) override;
	virtual UWorld* GetDefaultWorld() override { return GetWorld(); };
	//End INAssemblyOperationOwner

	/** @return true if the subsystem is tracking at least one active operation. */
	bool HasKnownOperation() const { return !KnownOperations.IsEmpty(); }
	/** Record a relay as the local-player relay used for client-side generation coordination. */
	void RegisterLocalRelay(ANWorldAssemblyRelay* InRelay);
	/** Drop the local-player relay reference when the relay is being torn down. */
	void UnregisterLocalRelay(const ANWorldAssemblyRelay* InRelay);
	
	/** @return true if at least one organ is queued awaiting assembly. */
	bool HasQueuedOrgansForAssembly() const { return !QueuedOrgansForAssembly.IsEmpty(); }
	/** Queue an organ component to be assembled on a subsequent tick. */
	void RegisterOrganForAssembly(TObjectPtr<UNOrganComponent> Organ);

	/** Fired each time a new operation begins being tracked by the subsystem, immediately before its build is kicked off. */
	UPROPERTY(BlueprintAssignable)
	FNSimpleDynamicMulticastDelegate OnOperationStarted;

	/** Fired when the last tracked operation finishes or is destroyed, i.e. when the tracked set transitions from non-empty to empty. */
	UPROPERTY(BlueprintAssignable)
	FNSimpleDynamicMulticastDelegate OnOperationsCompleted;
	
	/** Fired at the end of Clear() once tracked operations have been canceled and all cell proxies in the world have been destroyed. */
	UPROPERTY(BlueprintAssignable)
	FNSimpleDynamicMulticastDelegate OnCleared;

	/** Spawn an ANWorldAssemblyRelay bound to PlayerController and store it in RelayMap. */
	UFUNCTION(BlueprintCallable, DisplayName="Spawn Relay", Category = "NEXUS|WorldAssembly", meta=(ToolTip="Helpful for seamless travel, on server to spawn relays after a travel."))
	void SpawnRelay(APlayerController* PlayerController);
	
private:
	
	/**
	 * Cached copy of UNWorldAssemblySettings::bSupportSeamlessTravel, captured on world begin play (authority only).
	 * When true, the GameMode OnPostLogin/OnLogout delegates are not bound (they do not reliably survive seamless
	 * travel); instead the subsystem stays tickable and re-runs EnsurePlayerControllerRelays every Tick to keep each
	 * player's relay in sync. When false, those delegates manage relays and this per-tick monitor stays off.
	 */
	bool bCachedSeamlessTravelMonitor = false;
	
	/** Operations currently known to the subsystem; held strong to keep them alive across their build. */
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	UPROPERTY()
	TArray<TObjectPtr<UNAssemblyOperation>> KnownOperations;
	
	/**
	 * Externally-owned actors enrolled via RegisterActorForCleanup that should be destroyed on the next Clear() pass.
	 * Held weakly so entries become inert (rather than dangling) if the actor is destroyed by another system first.
	 */
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> TrackedActorsForCleanup;

	/**
	 * Organs waiting to be folded into an assembly operation.
	 * Populated by RegisterOrganForAssembly (called by UNOrganComponent on begin play) and drained each tick
	 * into a freshly-created UNAssemblyOperation; the presence of entries keeps the subsystem tickable.
	 */
	TArray<TObjectPtr<UNOrganComponent>> QueuedOrgansForAssembly;
	
	/** Used to track potential operations that will cache data, so that we can clear it. **/
	TArray<int32> CachedOperationTickets;

	/** Map from player controller to the relay actor spawned for that player. */
	UPROPERTY()
	TMap<TObjectPtr<APlayerController>, TObjectPtr<ANWorldAssemblyRelay>> RelayMap;

	/** Relay owned by the locally controlled player, if any. */
	UPROPERTY()
	TObjectPtr<ANWorldAssemblyRelay> LocalRelay;

	/** Handle for the OnPostLogin game-mode delegate so it can be cleanly unbound. */
	FDelegateHandle OnLoginHandle;
	/** Handle for the OnLogout game-mode delegate so it can be cleanly unbound. */
	FDelegateHandle OnLogoutHandle;

	/** GameMode::OnPostLogin handler — spawns the per-player relay for NewPlayer. */
	void OnPostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer);
	/** GameMode::OnLogout handler — destroys the relay associated with Exiting, if any. */
	void OnLogout(AGameModeBase* GameMode, AController* Exiting);
	/** Destroy the relay previously spawned for PlayerController and remove it from RelayMap. */
	void DestroyRelay(APlayerController* PlayerController);
	
	
	/**
	 * Back-fill relays for player controllers that already exist in InWorld, spawning one for any not yet in RelayMap.
	 * Complements the OnPostLogin path (which only fires for players joining afterward) to cover controllers present
	 * before the relay machinery is wired up; SpawnRelay is idempotent, so already-served controllers are skipped.
	 * This is also useful in solving the problem of seamless travel.
	 */
	void EnsurePlayerControllerRelays(const UWorld* InWorld);
};
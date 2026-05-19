// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NActorLibrary.h"
#include "Assembly/INAssemblyOperationOwner.h"
#include "Macros/NSubsystemMacros.h"
#include "Assembly/NAssemblyOperationSettings.h"
#include "NWorldAssemblyRelay.h"
#include "NWorldAssemblySubsystem.generated.h"

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

	/** @return Relay associated with the local player, or nullptr if it has not yet been spawned. */
	UFUNCTION(BlueprintCallable, DisplayName="Get Local Relay", Category = "NEXUS|WorldAssembly")
	ANWorldAssemblyRelay* GetLocalRelay() const { return LocalRelay; }

	/**
	 * @return true when the local procgen view is settled relative to the server.
	 * @remark Server path: no operations are currently in flight. Client path: LocalRelay has replicated, the nearby-cell payload has been received, and no operations the client has been notified of are pending.
	 * @note Does not gate Generate() — that can be called at any time regardless of this value.
	 */
	UFUNCTION(blueprintCallable, DisplayName="Is Ready?", Category = "NEXUS|WorldAssembly")
	bool IsReady();

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
	bool HasKnownOperation() const { return KnownOperations.Num() > 0; }
	/** Record a relay as the local-player relay used for client-side generation coordination. */
	void RegisterLocalRelay(ANWorldAssemblyRelay* InRelay);
	/** Drop the local-player relay reference when the relay is being torn down. */
	void UnregisterLocalRelay(const ANWorldAssemblyRelay* InRelay);
	
	/** Fired when the last tracked operation finishes or is destroyed, i.e. when the tracked set transitions from non-empty to empty. */
	UPROPERTY(BlueprintAssignable)
	FNSimpleDynamicMulticastDelegate OnOperationsCompleted;

private:
	/** Operations currently known to the subsystem; held strong to keep them alive across their build. */
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	UPROPERTY()
	TArray<TObjectPtr<UNAssemblyOperation>> KnownOperations;

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
	/** Spawn an ANWorldAssemblyRelay bound to PlayerController and store it in RelayMap. */
	void SpawnRelay(APlayerController* PlayerController);
	/** Destroy the relay previously spawned for PlayerController and remove it from RelayMap. */
	void DestroyRelay(APlayerController* PlayerController);
};
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "INProcGenOperationOwner.h"
#include "Macros/NSubsystemMacros.h"
#include "NProcGenOperationSettings.h"
#include "NProcGenRelay.h"
#include "NProcGenSubsystem.generated.h"

class ANProcGenRelay;
class UNProcGenOperation;
class FNProcGenTaskGraph;
class ANCellProxy;
class ANCellActor;
class UNProcGenContext;
class APlayerController;
class AController;
class AGameModeBase;


/**
 * Runtime world subsystem that drives ProcGen generation during gameplay.
 *
 * Game-only; also implements INProcGenOperationOwner so it can host operations created via Generate().
 * Spawns one ANProcGenRelay per connected player controller to carry per-player generation state.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | ProcGen Subsystem")
class NEXUSPROCGEN_API UNProcGenSubsystem : public UTickableWorldSubsystem, public INProcGenOperationOwner
{
	GENERATED_BODY()
	N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY(UNProcGenSubsystem, true)

public:
	/**
	 * Kick off a new generation pass with the supplied per-operation settings.
	 * @param Settings Operation-level settings (seed, level-instance behaviour); taken by reference so the caller can reuse the struct.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Generate", Category = "NEXUS|ProcGen")
	void Generate(UPARAM(ref) FNProcGenOperationSettings& Settings);

	/** @return Relay associated with the local player, or nullptr if it has not yet been spawned. */
	UFUNCTION(BlueprintCallable, DisplayName="Get Local Relay", Category = "NEXUS|ProcGen")
	ANProcGenRelay* GetLocalRelay() const { return LocalRelay; }

	/** @return true once the subsystem has finished world-begin-play setup and can accept Generate calls. */
	UFUNCTION(blueprintCallable, DisplayName="Is Ready?", Category = "NEXUS|ProcGen")
	bool IsReady();

	//~UTickableWorldSubsystem
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
	N_TICKABLE_WORLD_SUBSYSTEM_GET_TICKABLE_TICK_TYPE(ETickableTickType::Conditional)
	//End UTickableWorldSubsystem

	//~INProcGenOperationOwner
	virtual void StartOperation(UNProcGenOperation* Operation) override;
	virtual void OnOperationFinished(UNProcGenOperation* Operation, TSharedRef<FNProcGenTaskGraphContext> TaskGraphContext) override;
	virtual void OnOperationDestroyed(UNProcGenOperation* Operation) override;
	virtual UWorld* GetDefaultWorld() override { return GetWorld(); };
	//End INProcGenOperationOwner

	/** @return true if the subsystem is tracking at least one active operation. */
	bool HasKnownOperation() const { return KnownOperations.Num() > 0; }
	/** Record a relay as the local-player relay used for client-side generation coordination. */
	void RegisterLocalRelay(ANProcGenRelay* InRelay);
	/** Drop the local-player relay reference when the relay is being torn down. */
	void UnregisterLocalRelay(const ANProcGenRelay* InRelay);

private:
	/** Operations currently known to the subsystem; held strong to keep them alive across their build. */
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	UPROPERTY()
	TArray<TObjectPtr<UNProcGenOperation>> KnownOperations;

	/** Map from player controller to the relay actor spawned for that player. */
	UPROPERTY()
	TMap<TObjectPtr<APlayerController>, TObjectPtr<ANProcGenRelay>> RelayMap;

	/** Relay owned by the locally controlled player, if any. */
	UPROPERTY()
	TObjectPtr<ANProcGenRelay> LocalRelay;

	/** Handle for the OnPostLogin game-mode delegate so it can be cleanly unbound. */
	FDelegateHandle OnLoginHandle;
	/** Handle for the OnLogout game-mode delegate so it can be cleanly unbound. */
	FDelegateHandle OnLogoutHandle;

	void OnPostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer);
	void OnLogout(AGameModeBase* GameMode, AController* Exiting);
	void SpawnRelay(APlayerController* PlayerController);
	void DestroyRelay(APlayerController* PlayerController);
};
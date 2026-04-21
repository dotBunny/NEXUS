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


UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | ProcGen Subsystem")
class NEXUSPROCGEN_API UNProcGenSubsystem : public UTickableWorldSubsystem, public INProcGenOperationOwner
{
	GENERATED_BODY()
	N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY(UNProcGenSubsystem, true)
	
public:
	
	UFUNCTION(BlueprintCallable, DisplayName="Generate", Category = "NEXUS|ProcGen")
	void Generate(UPARAM(ref) FNProcGenOperationSettings& Settings);
	
	UFUNCTION(BlueprintCallable, DisplayName="Get Local Relay", Category = "NEXUS|ProcGen")
	ANProcGenRelay* GetLocalRelay() const { return LocalRelay; }
	
	UFUNCTION(blueprintCallable, DisplayName="Has Initial NCellLevelInstances?", Category = "NEXUS|ProcGen")
	bool HasInitialCellLevelInstances();
	
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	N_TICKABLE_WORLD_SUBSYSTEM_GET_TICKABLE_TICK_TYPE(ETickableTickType::Conditional)

	virtual void StartOperation(UNProcGenOperation* Operation) override;
	virtual void OnOperationFinished(UNProcGenOperation* Operation, TSharedRef<FNProcGenTaskGraphContext> TaskGraphContext) override;
	virtual void OnOperationDestroyed(UNProcGenOperation* Operation) override;
	virtual UWorld* GetDefaultWorld() override { return GetWorld(); };
	
	bool HasKnownOperation() const { return KnownOperations.Num() > 0; }
	void RegisterLocalRelay(ANProcGenRelay* InRelay);
	void UnregisterLocalRelay(const ANProcGenRelay* InRelay);

private:

	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	UPROPERTY()
	TArray<TObjectPtr<UNProcGenOperation>> KnownOperations;

	UPROPERTY()
	TMap<TObjectPtr<APlayerController>, TObjectPtr<ANProcGenRelay>> RelayMap;

	UPROPERTY()
	TObjectPtr<ANProcGenRelay> LocalRelay;

	FDelegateHandle OnLoginHandle;
	FDelegateHandle OnLogoutHandle;

	void OnPostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer);
	void OnLogout(AGameModeBase* GameMode, AController* Exiting);
	void SpawnRelay(APlayerController* PlayerController);
	void DestroyRelay(APlayerController* PlayerController);
};
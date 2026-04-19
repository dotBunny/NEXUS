// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "INProcGenOperationOwner.h"
#include "Macros/NSubsystemMacros.h"
#include "NProcGenOperationSettings.h"
#include "Math/NSeedGenerator.h"
#include "NProcGenSubsystem.generated.h"

class UNProcGenOperation;
class FNProcGenTaskGraph;
class ANCellProxy;
class ANCellActor;
class UNProcGenContext;


UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | ProcGen Subsystem")
class NEXUSPROCGEN_API UNProcGenSubsystem : public UTickableWorldSubsystem, public INProcGenOperationOwner
{
	GENERATED_BODY()
	N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY(UNProcGenSubsystem, true)
	
public:
	
	UFUNCTION(BlueprintCallable, DisplayName="Generate", Category = "NEXUS|ProcGen")
	void Generate(UPARAM(ref) FNProcGenOperationSettings& Settings);
	
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	
	N_TICKABLE_WORLD_SUBSYSTEM_GET_TICKABLE_TICK_TYPE(ETickableTickType::Conditional)

	virtual void StartOperation(UNProcGenOperation* Operation) override;
	virtual void OnOperationFinished(UNProcGenOperation* Operation, TSharedRef<FNProcGenOperationSharedContext> SharedContext) override;
	virtual void OnOperationDestroyed(UNProcGenOperation* Operation) override;
	virtual UWorld* GetDefaultWorld() override { return GetWorld(); };
	
	bool HasKnownOperation() const { return KnownOperations.Num() > 0; }
	
private:	
	
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	UPROPERTY()
	TArray<TObjectPtr<UNProcGenOperation>> KnownOperations;
	
	
	UPROPERTY()
	TArray<TObjectPtr<ANCellActor>> KnownCellActors;
	
	UPROPERTY()
	TArray<TObjectPtr<ANCellProxy>> KnownCellProxies;
};
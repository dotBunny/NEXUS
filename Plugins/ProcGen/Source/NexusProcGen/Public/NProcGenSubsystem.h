// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NSubsystemMacros.h"
#include "NProcGenSubsystem.generated.h"

class UNProcGenOperation;
class FNProcGenOperationTaskGraph;
class ANCellProxy;
class ANCellActor;
class UNProcGenContext;

UCLASS()
class NEXUSPROCGEN_API UNProcGenSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY(UNProcGenSubsystem, true)

public:
	
	bool RegisterCellActor(ANCellActor* CellActor);
	bool UnregisterCellActor(ANCellActor* CellActor);

	bool RegisterCellProxy(ANCellProxy* CellProxy);
	bool UnregisterCellProxy(ANCellProxy* CellProxy);
	
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	
	void StartOperation(UNProcGenOperation* Operation);
	void OnOperationFinished(UNProcGenOperation* Operation);
	bool HasKnownOperation() const { return KnownOperations.Num() > 0; }
	
	
private:	
	
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TArray<UNProcGenOperation*> KnownOperations;
	
	TArray<ANCellActor*> KnownCellActors;
	TArray<ANCellProxy*> KnownCellProxies;
};
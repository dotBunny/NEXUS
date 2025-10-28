// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NSubsystemMacros.h"
#include "NProcGenSubsystem.generated.h"

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

private:	
	TArray<ANCellActor*> KnownCellActors;
	TArray<ANCellProxy*> KnownCellProxies;
};
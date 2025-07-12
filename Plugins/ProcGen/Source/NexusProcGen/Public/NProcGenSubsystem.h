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
	
	bool RegisterNCellActor(ANCellActor* NCellActor);
	bool UnregisterNCellActor(ANCellActor* NCellActor);

	bool RegisterNCellProxy(ANCellProxy* NCellProxy);
	bool UnregisterNCellProxy(ANCellProxy* NCellProxy);

private:	
	TArray<ANCellActor*> KnownNCellActors;
	TArray<ANCellProxy*> KnownNCellProxies;
};
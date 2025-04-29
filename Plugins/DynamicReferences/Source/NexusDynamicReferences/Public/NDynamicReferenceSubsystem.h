// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDynamicReference.h"
#include "Macros/NSubsystemMacros.h"
#include "NDynamicReferenceSubsystem.generated.h"


UCLASS(BlueprintType)
class NEXUSDYNAMICREFERENCES_API UNDynamicReferenceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	N_WORLD_SUBSYSTEM_GAME_ONLY(UNDynamicReferenceSubsystem, true)
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void RegisterReference(ENDynamicReference InType, AActor* InActor);
	
	void UnregisterReference(ENDynamicReference InType, AActor* InActor);

	UFUNCTION(BlueprintCallable)
	TArray<AActor*>& GetReferences(const ENDynamicReference InType)
	{
		return ReferenceMap[InType];
	}

private:
	TArray<TArray<AActor*>> ReferenceMap;
};
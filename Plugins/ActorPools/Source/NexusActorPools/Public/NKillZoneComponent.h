// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "NKillZoneComponent.generated.h"

class UNActorPoolSubsystem;

/**
 * A classic kill plane implementation with support for pooled actors.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class NEXUSACTORPOOLS_API UNKillZoneComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	explicit UNKillZoneComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Actor Pools")
	int GetKillCount() const { return KillCount; }
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Actor Pools")
	void SetKillCount(const int NewKillCount) { KillCount = NewKillCount; }

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Actor Pools")
	void ResetKillCount() { KillCount = 0; }

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);
	
protected:
	/**
	 * Should static/station actors be ignored?
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	bool bDontDestroyStaticActors = true;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	bool bDontDestroyNonInterfacedActors = false;
	
private:
	int KillCount = 0;
	UNActorPoolSubsystem* ActorPoolSubsystem;
};
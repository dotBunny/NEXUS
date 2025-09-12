// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "NKillZoneComponent.generated.h"

class UNActorPoolSubsystem;

/**
 * A kill plane implementation built to automatically pool properly configured AActor upon overlap.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/types/kill-zone-component/">NEXUS.Docs</a>
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class NEXUSACTORPOOLS_API UNKillZoneComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	explicit UNKillZoneComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, DisplayName="Get Kill Count", Category = "NEXUS|Actor Pools",
		meta=(ToolTip="Gets the internal counter tracking the number of AActors the component has killed.",
		DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/kill-zone-component/#get-kill-count"))
	int32 GetKillCount() const { return KillCount; }
	
	UFUNCTION(BlueprintCallable, DisplayName="Set Kill Count", Category = "NEXUS|Actor Pools",
		meta=(ToolTip="Sets the internal counter tracking the number of AActors the component has killed.",
		DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/kill-zone-component/#set-kill-count"))
	void SetKillCount(const int32 NewKillCount) { KillCount = NewKillCount; }

	UFUNCTION(BlueprintCallable, DisplayName="Reset Kill Count", Category = "NEXUS|Actor Pools",
		meta=(ToolTip="Resets the internal counter tracking the number of AActors the component has killed to 0.",
		DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/kill-zone-component/#reset-kill-count"))
	void ResetKillCount() { KillCount = 0; }

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);
	
protected:
	/**
	 * Should static/station actors be ignored?
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, DisplayName="Ignore Static Actors", Category="Kill Zone")
	bool bIgnoreStaticActors = true;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, DisplayName="Ignore Non-INActorPoolItem Actors", Category="Kill Zone")
	bool bIgnoreNonInterfacedActors = false;
	
private:
	int32 KillCount = 0;
	UNActorPoolSubsystem* ActorPoolSubsystem;
};
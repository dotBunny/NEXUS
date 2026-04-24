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
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/types/kill-zone-component/">UNKillZoneComponent</a>
 */
UCLASS(ClassGroup="NEXUS", DisplayName = "NEXUS | Kill Zone", meta = (BlueprintSpawnableComponent))
class NEXUSACTORPOOLS_API UNKillZoneComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	explicit UNKillZoneComponent(const FObjectInitializer& ObjectInitializer);

	//~UActorComponent
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//End UActorComponent

	/**
	 * Gets the internal counter tracking the number of AActors the component has killed.
	 * @return The kill count.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Kill Count", Category = "NEXUS|Actor Pools",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/kill-zone-component/#get-kill-count"))
	int32 GetKillCount() const { return KillCount; }

	/**
	 * Sets the internal counter tracking the number of AActors the component has killed.
	 * @param NewKillCount The new value to use as the kill count.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Set Kill Count", Category = "NEXUS|Actor Pools",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/kill-zone-component/#set-kill-count"))
	void SetKillCount(const int32 NewKillCount) { KillCount = NewKillCount; }

	/** Resets the internal counter tracking the number of AActors the component has killed to 0. */
	UFUNCTION(BlueprintCallable, DisplayName="Reset Kill Count", Category = "NEXUS|Actor Pools",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/kill-zone-component/#reset-kill-count"))
	void ResetKillCount() { KillCount = 0; }

	/**
	 * Overlap handler that returns the overlapping Actor to its Actor Pool (or destroys it when appropriate).
	 * @param OverlappedComponent This component.
	 * @param OtherActor The Actor that entered the kill zone.
	 * @param OtherComp The primitive component from OtherActor that overlapped.
	 * @param OtherBodyIndex The body index on OtherComp.
	 * @param bFromSweep True if the overlap was detected during a sweep.
	 * @param SweepHitResult The hit result if bFromSweep is true.
	 */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

protected:

	/** When true, Actors with a Static mobility root component are ignored by the kill zone. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, DisplayName="Ignore Static Actors", Category="Kill Zone")
	bool bIgnoreStaticActors = true;

	/** When true, Actors that do not implement INActorPoolItem are ignored by the kill zone. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, DisplayName="Ignore Non-INActorPoolItem Actors", Category="Kill Zone")
	bool bIgnoreNonInterfacedActors = false;

private:
	/** Running count of Actors killed by this zone since spawn (or the last reset). */
	int32 KillCount = 0;

	/** Cached reference to the ActorPool subsystem used to route returns. */
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	UNActorPoolSubsystem* ActorPoolSubsystem;
};
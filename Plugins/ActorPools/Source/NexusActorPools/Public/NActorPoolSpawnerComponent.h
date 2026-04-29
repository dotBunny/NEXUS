// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NActorPoolSettings.h"
#include "NActorPoolSubsystem.h"
#include "Collections/NWeightedIntegerArray.h"
#include "NActorPoolSpawnerComponent.generated.h"

class USplineComponent;

/**
 * A single weighted entry in a UNActorPoolSpawnerComponent's template list.
 */
USTRUCT(BlueprintType)
struct NEXUSACTORPOOLS_API FNActorPoolSpawnerTemplate
{
	GENERATED_BODY()

	/** The Actor class to spawn. */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> Template;

	/** The pool settings associated with this Actor class. */
	UPROPERTY(EditAnywhere)
	FNActorPoolSettings Settings;

	/** The relative weight of this entry when the spawner selects a template. */
	UPROPERTY(EditAnywhere)
	int32 Weight = 1;
};

/**
 * Spatial distribution used by a UNActorPoolSpawnerComponent to choose a per-spawn location.
 */
UENUM(BlueprintType)
enum class ENActorPoolSpawnerDistribution : uint8
{
	/** Spawn exactly at the component's offset origin. */
	Point = 0,
	/** Spawn on the perimeter of a 2D circle around the origin. */
	Radius = 1,
	/** Spawn anywhere inside a sphere around the origin. */
	Sphere = 2,
	/** Spawn anywhere inside an axis-aligned box around the origin. */
	Box = 3,
	/** Spawn along a referenced spline component. */
	Spline = 4,
	/** Spawn anywhere inside a box rotated by the component's rotation. */
	OrientedBox = 5
};

/**
 * A fundamental spawning component which will interact with the UNActorPoolSubsystem to periodically spawn defined AActors in predefined distributions (shapes).
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-spawner-component/">UNActorPoolSpawnerComponent</a>
 */
UCLASS(ClassGroup="NEXUS", DisplayName = "NEXUS | Actor Pool Spawner", meta = (BlueprintSpawnableComponent),
	HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, HLOD, LOD, Rendering, Collision, Physics))
class NEXUSACTORPOOLS_API UNActorPoolSpawnerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	/** Is the component going to Spawn enemies when ticked? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	bool bSpawningEnabled = true;
	
	/** Should the spawner only spawn on servers, ignoring itself on client-only? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	bool bServerAuthoritative = true;

	/** The rate at which things should be spawned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	float SpawnRate = 0.5f;

	/** Offset from the component location to treat as the origin when calculating a position to spawn an actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	FVector Offset = FVector::Zero();

	/** The distribution method used to choose where things are spawned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	ENActorPoolSpawnerDistribution Distribution = ENActorPoolSpawnerDistribution::Point;

	/** The dimensional input to the distribution method. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	FVector DistributionRange = FVector(1.f, 20.f, 20.f);
	
	/** The in-level component reference for usage with the Spline distribution method. */
	UPROPERTY(EditAnywhere, meta = (UseComponentPicker, AllowedClasses = "/Script/Engine.SplineComponent", AllowAnyActor, EditCondition="Distribution == ENActorPoolSpawnerDistribution::Spline", EditConditionHides), Category="Actor Pool Spawner")
	FComponentReference SplineLevelReference;

	/** If you want to reference a Spline in a BP with the ActorPoolSpawner, you will need to enter its name here, and it will be linked during BeginPlay. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Actor Pool Spawner")
	FName SplineComponentName;

	/** The number of items to spawn at any given spawn event. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	int32 Count = 1;

	/** Should we randomize the seed on BeginPlay? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	bool bRandomizeSeed = false;

	/** Seed used when determining what to pull from the weighted list. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	int32 Seed = 0;

	explicit UNActorPoolSpawnerComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Initiate a spawn call for the component, ignoring any timers.
	 * @param bIgnoreSpawningFlag Should the internal spawning flag state be ignored?
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Spawn", Category = "NEXUS|Actor Pools",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-spawner-component/#spawn"))
	void Spawn(bool bIgnoreSpawningFlag = false);

	/** Disables the component's internal flag to stop any spawning from occuring. */
	UFUNCTION(BlueprintCallable, DisplayName="Disable Spawning", Category = "NEXUS|Actor Pools",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-spawner-component/#disable-spawning"))
	void DisableSpawning()
	{
		bSpawningEnabled = false;
	}

	/** Enables the component's internal flag to allow spawning to occur (on by default). */
	UFUNCTION(BlueprintCallable, DisplayName="Enable Spawning", Category = "NEXUS|Actor Pools",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-spawner-component/#enable-spawning"))
	void EnableSpawning()
	{
		bSpawningEnabled = true;
	}
	
	/** @return The per-spawn origin offset from the component's world location. */
	FORCEINLINE FVector GetOffset() const { return Offset; }
	/** @return The spatial distribution used for picking spawn locations. */
	FORCEINLINE ENActorPoolSpawnerDistribution GetDistribution() const { return Distribution; }
	/** @return The effective distribution extents, scaled by the component's world scale. */
	FORCEINLINE FVector GetDistributionRange() const { return DistributionRange * GetComponentScale(); }
	/** @return The rotation applied to the distribution shape (the component's world rotation). */
	FORCEINLINE FRotator GetDistributionRotation() const { return GetComponentRotation(); }

protected:
	/** Weighted list of Actor templates the spawner will choose from at each spawn event. */
	UPROPERTY(EditAnywhere, Meta = (AllowPrivateAccess = "true", TitleProperty="{Template}"), Category="Actor Pool Spawner")
	TArray<FNActorPoolSpawnerTemplate> Templates;
private:
	void CacheSplineComponent();

	/** Precomputed weighted index sampler built from Templates. */
	FNWeightedIntegerArray WeightedIndices;

	/** Whether this component is running on the network authority (for bServerAuthoritative). */
	bool bIsNetAuthority = false;

	/** Accumulated time since the last spawn event, used against SpawnRate. */
	float TimeSinceSpawned = 0.f;

	/** Cached reference to the owning world's UNActorPoolSubsystem. */
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	TObjectPtr<UNActorPoolSubsystem> Manager = nullptr;

	/** Cached count of Templates for tight loops. */
	int32 TemplateCount = 0;

	/** Cached spline component used when Distribution == Spline. */
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	TObjectPtr<USplineComponent> CachedSplineComponent = nullptr;
};
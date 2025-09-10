// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NActorPoolSettings.h"
#include "NActorPoolSubsystem.h"
#include "Collections/NWeightedIntegerArray.h"
#include "NActorPoolSpawnerComponent.generated.h"

class UActorPoolSubsystem;
class USplineComponent;

USTRUCT(BlueprintType)
struct NEXUSACTORPOOLS_API FNActorPoolSpawnerTemplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> Template;
	UPROPERTY(EditAnywhere)
	FNActorPoolSettings Settings;
	UPROPERTY(EditAnywhere)
	int32 Weight = 1;
};

UENUM(BlueprintType)
enum ENActorPoolSpawnerDistribution : uint8
{
	APSD_Point			UMETA(DisplayName = "Point"),
	APSD_Radius			UMETA(DisplayName = "Radius"),
	APSD_Sphere			UMETA(DisplayName = "Sphere"),
	APSD_Box			UMETA(DisplayName = "Box"),
	APSD_Spline			UMETA(DisplayName = "Spline")
};

/**
 * A fundamental spawning component which will interact with the UNActorPoolSubsystem to periodically spawn defined AActors in predefined distributions (shapes).
 */
UCLASS(meta = (BlueprintSpawnableComponent),
	HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, HLOD, LOD, Rendering, Collision, Physics))
class NEXUSACTORPOOLS_API UNActorPoolSpawnerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	/**
	 * Is the component going to Spawn enemies when ticked?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	bool bSpawningEnabled = true;
	
	/**
	 * Should the spawner only spawn on servers, ignoring itself on client-only.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	bool bServerAuthoritative = true;

	/**
	 * The rate at which things should be spawned.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	float SpawnRate = 0.5f;

	/**
	 * Offset from the component location to treat as the origin when calculating a position to spawn an actor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	FVector Offset = FVector::Zero();

	/**
	* How should the point be chosen where things are spawned.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	TEnumAsByte<ENActorPoolSpawnerDistribution> Distribution = ENActorPoolSpawnerDistribution::APSD_Point;

	/**
	* How should the point be chosen where things are spawned.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	FVector DistributionRange = FVector(1.f, 20.f, 20.f);

	/**
	 * The in-level component reference for usage with the APSD_Spline for Distribution.
	 */
	UPROPERTY(EditAnywhere, meta = (UseComponentPicker, AllowedClasses = "/Script/Engine.SplineComponent", AllowAnyActor, EditCondition="Distribution == ENActorPoolSpawnerDistribution::APSD_Spline", EditConditionHides), Category="Actor Pool Spawner")
	FComponentReference SplineLevelReference;

	/**
	 * If you want to reference a Spline in a BP with the ActorPoolSpawner, you will need to enter its name here and it will be linked during BeginPlay.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Actor Pool Spawner")
	FName SplineComponentName;

	/**
	 * The number of items to spawn at any given spawn event.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	int32 Count = 1;

	/**
	 * Should we randomize the seed on begin play?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	bool bRandomizeSeed = false;

	/**
	 * Seed used when determining what to pull from the weighted list.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor Pool Spawner")
	int32 Seed = 0;

	explicit UNActorPoolSpawnerComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Actor Pools")
	void Spawn(bool bIgnoreSpawningFlag = false);

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Actor Pools")
	void DisableSpawning()
	{
		bSpawningEnabled = false;
	}

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Actor Pools")
	void EnableSpawning()
	{
		bSpawningEnabled = true;
	}
	
	FORCEINLINE FVector GetOffset() const { return Offset; }
	FORCEINLINE ENActorPoolSpawnerDistribution GetDistribution() const { return Distribution; }
	FORCEINLINE FVector GetDistributionRange() const { return DistributionRange; }

protected:
	UPROPERTY(EditAnywhere, Meta = (AllowPrivateAccess = "true", TitleProperty="{Template}"), Category="Actor Pool Spawner")
	TArray<FNActorPoolSpawnerTemplate> Templates;
private:
	FNWeightedIntegerArray WeightedIndices;
	bool bIsNetAuthority = false;
	float TimeSinceSpawned = 0.f;
	UNActorPoolSubsystem* Manager = nullptr;
	int32 TemplateCount = 0;
	USplineComponent* CachedSplineComponent = nullptr;
};
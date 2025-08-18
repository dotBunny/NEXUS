// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "NActorPoolSettings.h"
#include "NActorPoolSubsystem.h"
#include "Templates/Tuple.h"
#include "Collections/NWeightedIntegerArray.h"
#include "Components/SplineComponent.h"
#include "NActorPoolSpawnerComponent.generated.h"

class UActorPoolSubsystem;

USTRUCT(BlueprintType)
struct NEXUSACTORPOOLS_API FNActorPoolSpawnerTemplate
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly)
	TSubclassOf<AActor> Template;
	UPROPERTY(EditInstanceOnly)
	FNActorPoolSettings Settings;
	UPROPERTY(EditInstanceOnly)
	int Weight = 1;
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

// TODO: Toggle option to be determinisitc?
UCLASS(meta = (BlueprintSpawnableComponent),
	HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, HLOD, LOD, Rendering, Collision, Physics))
class NEXUSACTORPOOLS_API UNActorPoolSpawnerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	/**
	 * Is the component going to Spawn enemies when ticked?
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	bool bSpawningEnabled = true;
	
	/**
	 * Should the spawner only spawn on servers, ignoring itself on client-only.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	bool bServerAuthoritative = true;

	/**
	 * The rate at which things should be spawned.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnRate = 0.5f;

	/**
	 * Offset from the component location to treat as the origin when calculating a position to spawn an actor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Offset = FVector::Zero();

	/**
	* How should the point be chosen where things are spawned.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ENActorPoolSpawnerDistribution> Distribution = ENActorPoolSpawnerDistribution::APSD_Point;

	/**
	* How should the point be chosen where things are spawned.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector DistributionRange = FVector(1.f, 20.f, 20.f);

	/**
	 * The in-level component reference for usage with the APSD_Spline for Distribution.
	 */
	UPROPERTY(EditInstanceOnly, meta = (UseComponentPicker, AllowedClasses = "/Script/Engine.SplineComponent", AllowAnyActor, EditCondition="Distribution == ENActorPoolSpawnerDistribution::APSD_Spline", EditConditionHides))
	FComponentReference SplineLevelReference;

	/**
	 * If you want to reference a Spline in a BP with the ActorPoolSpawner, you will need to enter its name here and it will be linked during BeginPlay.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName SplineComponentName;

	/**
	 * The number of items to spawn at any given spawn event.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Count = 1;

	/**
	 * Should we randomize the seed on begin play?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRandomizeSeed = false;

	/**
	 * Seed used when determining what to pull from the weighted list.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	int Seed = 0;

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
	UPROPERTY(EditInstanceOnly, Meta = (AllowPrivateAccess = "true", TitleProperty="{Template}"))
	TArray<FNActorPoolSpawnerTemplate> Templates;
private:
	FNWeightedIntegerArray WeightedIndices;
	bool bIsNetAuthority = false;
	float TimeSinceSpawned = 0.f;
	UNActorPoolSubsystem* Manager = nullptr;
	int TemplateCount = 0;
	USplineComponent* CachedSplineComponent = nullptr;
};
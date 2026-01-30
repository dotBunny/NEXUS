// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolSpawnerComponent.h"
#include "NActorPoolSubsystem.h"
#include "NBoxPicker.h"
#include "NCirclePicker.h"
#include "NCoreMinimal.h"
#include "NSpherePicker.h"
#include "NSplinePicker.h"
#include "Components/SplineComponent.h"

UNActorPoolSpawnerComponent::UNActorPoolSpawnerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// Only the server will have this
	SetIsReplicatedByDefault(false);
	bAutoActivate = false;
	bNeverNeedsRenderUpdate = true;
}

void UNActorPoolSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only run on a server
	if (bServerAuthoritative && IsNetMode(NM_Client))
	{
		return;
	}

	// Register with the management system so it will handle ticking the component
	Manager = UNActorPoolSubsystem::Get(GetWorld());
	Manager->RegisterTickableSpawner(this);
	WeightedIndices.Empty();

	// Create / validate pools
	TemplateCount = Templates.Num();
	for (int32 i = 0; i < TemplateCount; i++)
	{
		if (Templates[i].Template == nullptr)
		{
			UE_LOG(LogNexusActorPools, Warning, TEXT("Detected an invalid template(NULL) at index(%d) as part of the pre-configured templates for the UNActorPoolSpawnerComponent owned by AActor(%s); skipping creating and adding to weighted indices."), i,  *this->GetOwner()->GetName());
			continue;
		}

		// Ensure actor pool is made, reuse if exists
		Manager->CreateActorPool(Templates[i].Template, Templates[i].Settings);

		// Add to the weighted list
		WeightedIndices.Add(i, Templates[i].Weight);
	}

	
	if (TemplateCount > 0 && bRandomizeSeed)
	{
		Seed = FNRandom::NonDeterministic.RandHelper(42);
	}

	if (Distribution == ENActorPoolSpawnerDistribution::Spline)
	{
		CacheSplineComponent();
	}
}


void UNActorPoolSpawnerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (Manager != nullptr) // !Client
	{
		Manager->UnregisterTickableSpawner(this);
	}
}

void UNActorPoolSpawnerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	TimeSinceSpawned += DeltaTime;
	if (TimeSinceSpawned < SpawnRate) return;
	Spawn();
}

void UNActorPoolSpawnerComponent::Spawn(const bool bIgnoreSpawningFlag)
{
	if (!bSpawningEnabled && !bIgnoreSpawningFlag || !WeightedIndices.HasData()) return;
	
	const FRotator SpawnRotator = this->GetComponentRotation();
	const FVector Origin = this->GetComponentLocation() + Offset;

	// NEW API
	TArray<FVector> OutLocations;
	if (Distribution == ENActorPoolSpawnerDistribution::Box)
	{
		FVector HalfDistribution = DistributionRange / 2.0f;
		FNBoxPickerParams Params;
		Params.Origin = Origin;
		Params.MaximumDimensions = FBox(-HalfDistribution, HalfDistribution);
		Params.Count = Count;
		FNBoxPicker::Tracked(OutLocations, Seed, Params);
	}
	else if (Distribution == ENActorPoolSpawnerDistribution::Radius)
	{
		FNCirclePickerParams Params;
		Params.Origin = Origin;
		Params.MinimumRadius = DistributionRange.X;
		Params.MaximumRadius = DistributionRange.Y;
		Params.Count = Count;
		FNCirclePicker::Tracked(OutLocations, Seed, Params);
	}
	
	
	for (int32 i = 0; i < Count; i++)
	{
		FVector SpawnLocation = Origin;
		int32 RandomIndex = 0;
		if (TemplateCount > 1)
		{
			RandomIndex = WeightedIndices.RandomTrackedValue(Seed);
		}
		switch (Distribution)
		{
			using enum ENActorPoolSpawnerDistribution;
		case Radius:
			SpawnLocation = OutLocations[i];break;
		case Sphere:
			FNSpherePicker::RandomTrackedPointInsideOrOn(Seed, SpawnLocation, Origin,DistributionRange.X, DistributionRange.Y);
			Seed++;
			break;
		case Box:
			SpawnLocation = OutLocations[i];
			break;
		case Spline:
			if (CachedSplineComponent != nullptr)
			{
				FNSplinePicker::RandomTrackedPointOn(Seed, SpawnLocation, CachedSplineComponent);
				Seed++;
			}
			break;
		default: ;
		}
		
		
		Manager->SpawnActor<AActor>(Templates[RandomIndex].Template, SpawnLocation, SpawnRotator);
	}
	TimeSinceSpawned = 0;
}

void UNActorPoolSpawnerComponent::CacheSplineComponent()
{
	// We have a level reference, first check
	if (!SplineLevelReference.PathToComponent.IsEmpty())
	{
		if (SplineLevelReference.OtherActor.IsValid())
		{
			AActor* Owner = SplineLevelReference.OtherActor.Get();
			CachedSplineComponent = Cast<USplineComponent>(SplineLevelReference.GetComponent(Owner));
		}
		else
		{
			CachedSplineComponent = Cast<USplineComponent>(SplineLevelReference.GetComponent(this->GetOwner()));
		}
	}
	else if (!SplineComponentName.IsNone())
	{
		// We have a name and should look for the component based off that
		TArray<USplineComponent*> FoundSplineComponents;
		this->GetOwner()->GetComponents<USplineComponent>(FoundSplineComponents, true);
		const int32 FoundCount = FoundSplineComponents.Num();
		for (int32 i = 0; i < FoundCount; i++)
		{
			if (FoundSplineComponents[i]->GetFName() == SplineComponentName)
			{
				CachedSplineComponent = FoundSplineComponents[i];
			}
		}
	}
	
	if (CachedSplineComponent == nullptr)
	{
		Distribution = ENActorPoolSpawnerDistribution::Point;
		UE_LOG(LogNexusActorPools, Error, TEXT("Unable to find USplineComponent to use for distribution with UNActorPoolSpawnerComponent on AActor(%s); changing to spawn at point."), *this->GetOwner()->GetName());
	}
}

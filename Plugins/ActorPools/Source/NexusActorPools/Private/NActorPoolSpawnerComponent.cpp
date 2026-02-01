// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolSpawnerComponent.h"
#include "NActorPoolSubsystem.h"
#include "NBoxPicker.h"
#include "NCirclePicker.h"
#include "NCoreMinimal.h"
#include "NOrientedBoxPicker.h"
#include "NOrientedBoxPickerParams.h"
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
	
	TArray<FVector> OutLocations;
	switch (Distribution)
	{
		using enum ENActorPoolSpawnerDistribution;
	case Radius:
		{
			FNCirclePickerParams Params;
			Params.Origin = this->GetComponentLocation() + Offset;
			Params.MinimumRadius = DistributionRange.X * GetComponentScale().X;
			Params.MaximumRadius = DistributionRange.Y * GetComponentScale().X;
			//Params.Rotation = DistributionRotation;
			Params.Count = Count;
			FNCirclePicker::Tracked(OutLocations, Seed, Params);
			break;
		}
	case Box:
		{
			FVector HalfDistribution = GetDistributionRange() / 2.0f;
			FNBoxPickerParams Params;
			Params.Origin = this->GetComponentLocation() + Offset;
			Params.MaximumDimensions = FBox(-HalfDistribution, HalfDistribution);
			Params.Count = Count;
			FNBoxPicker::Tracked(OutLocations, Seed, Params);
			break;
		}
	case Sphere:
		{
			FNSpherePickerParams Params;
			Params.Origin = this->GetComponentLocation() + Offset;
			Params.MinimumRadius = DistributionRange.X * GetComponentScale().X;
			Params.MaximumRadius = DistributionRange.Y * GetComponentScale().X;
			Params.Count = Count;
			FNSpherePicker::Tracked(OutLocations, Seed, Params);
			break;
		}
	case OrientedBox:
		{
			FNOrientedBoxPickerParams Params;
			Params.Origin = this->GetComponentLocation() + Offset;
			Params.MaximumDimensions = GetDistributionRange();
			Params.Rotation = GetDistributionRotation();
			Params.Count = Count;
			FNOrientedBoxPicker::Tracked(OutLocations, Seed, Params);
			break;
		}
	case Spline:
		if (CachedSplineComponent != nullptr)
		{
			FNSplinePickerParams Params;
			Params.SplineComponent = CachedSplineComponent;
			Params.Count = Count;
			FNSplinePicker::Tracked(OutLocations, Seed, Params);
		}
		else
		{
			UE_LOG(LogNexusActorPools, Error, TEXT("Unable to spawn actors as the USplineComponent is not valid."));
			return;
		}
		break;
	case Point:
		for (int32 i = 0; i < Count; i++)
		{
			OutLocations.Add(this->GetComponentLocation() + Offset);
		}
		break;
	default: 
		UE_LOG(LogNexusActorPools, Error, TEXT("Unable to spawn actors as the distribution type is not valid."));
		return;
	}
	
	const FRotator SpawnRotator = this->GetComponentRotation();
	for (int32 i = 0; i < Count; i++)
	{
		int32 RandomIndex = 0;
		if (TemplateCount > 1)
		{
			RandomIndex = WeightedIndices.RandomTrackedValue(Seed);
		}
		Manager->SpawnActor<AActor>(Templates[RandomIndex].Template, OutLocations[i], SpawnRotator);
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

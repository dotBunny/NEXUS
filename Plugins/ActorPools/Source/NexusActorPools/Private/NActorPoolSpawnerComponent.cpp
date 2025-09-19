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
			N_LOG(Warning, TEXT("[NActorPoolSpawnerComponent] Invalid template at index %d, skipping."), i);
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

	if (Distribution == APSD_Spline)
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
			Distribution = APSD_Point;
			N_LOG(Warning,
				TEXT("[NActorPoolSpawnerComponent] Unable to find SplineComponent to use for distribution on %s. Reverting to spawn at point."),
				*this->GetOwner()->GetName());

		}
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
		case APSD_Radius:
			FNCirclePicker::RandomTrackedPointInsideOrOn(Seed, SpawnLocation, Origin, DistributionRange.X, DistributionRange.Y);
			Seed++;
			break;
		case APSD_Sphere:
			FNSpherePicker::RandomTrackedPointInsideOrOn(Seed, SpawnLocation, Origin,DistributionRange.X, DistributionRange.Y);
			Seed++;
			break;
		case APSD_Box:
			FVector HalfDistribution = DistributionRange / 2.0f;
			FNBoxPicker::RandomTrackedPointInsideOrOnSimple(Seed, SpawnLocation, Origin, FBox(-HalfDistribution, HalfDistribution));
			Seed++;
			break;
		case APSD_Spline:
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

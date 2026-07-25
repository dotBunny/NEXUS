// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NActorPoolSpawnerComponent.h"
#include "Developer/NDebugActor.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

// Guards parity between where a spawner *draws* its Box distribution and where it *samples* it. The viewport
// visualizer (NActorPoolSpawnerComponentVisualizer.cpp, Box case) draws an FBox of half-extent
// GetDistributionRange() * 0.5 centered on the component location plus Offset; the picker
// (NActorPoolSpawnerComponent.cpp, Box case) samples FBox(-GetDistributionRange()/2, +GetDistributionRange()/2)
// about the same origin. Both must resolve to the same world-space box (including the component-scale fold that
// GetDistributionRange() applies) or the gizmo lies about where actors will appear.

N_TEST_HIGH(UNActorPoolSpawnerComponentTests_BoxDistribution_DrawnExtentsMatchSampledExtents,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSpawnerComponent::BoxDistribution::DrawnExtentsMatchSampledExtents",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies the drawn Box extents and the sampled Box extents resolve to the same world-space box under a
	// non-uniform component scale and a non-zero distribution offset.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		ANDebugActor* Owner = World->SpawnActor<ANDebugActor>();
		if (Owner == nullptr)
		{
			ADD_ERROR("Could not spawn owning actor for the spawner component.");
			return;
		}
		// Non-uniform scale on the root so GetComponentScale() (and the scale fold) is exercised per axis.
		Owner->SetActorScale3D(FVector(2.f, 3.f, 4.f));

		UNActorPoolSpawnerComponent* Spawner = NewObject<UNActorPoolSpawnerComponent>(Owner);
		if (Spawner == nullptr)
		{
			ADD_ERROR("Could not create the spawner component.");
			return;
		}
		Spawner->SetupAttachment(Owner->GetRootComponent());
		Spawner->RegisterComponent();

		Spawner->Distribution = ENActorPoolSpawnerDistribution::Box;
		Spawner->DistributionRange = FVector(100.f, 200.f, 300.f);
		Spawner->Offset = FVector(10.f, 20.f, 30.f);

		// GetDistributionRange() must fold the component scale in component-wise before either consumer halves it.
		const FVector Range = Spawner->GetDistributionRange();
		if (!Range.Equals(FVector(200.f, 600.f, 1200.f), 0.01f))
		{
			ADD_ERROR(FString::Printf(
				TEXT("GetDistributionRange() should fold component scale per axis; expected (200,600,1200) but got %s."),
				*Range.ToString()));
			return;
		}

		// Reproduce each consumer's math from the source of truth.
		const FVector DrawnHalfExtents = Range * 0.5f;   // visualizer Box case
		const FVector SampledHalfExtents = Range / 2.0f; // picker Box case
		if (!DrawnHalfExtents.Equals(SampledHalfExtents, UE_KINDA_SMALL_NUMBER))
		{
			ADD_ERROR(FString::Printf(TEXT("Box drawn half-extents %s diverge from sampled half-extents %s."),
				*DrawnHalfExtents.ToString(), *SampledHalfExtents.ToString()));
		}

		// Both consumers center the box on the component location plus the distribution Offset.
		const FVector Origin = Spawner->GetComponentLocation() + Spawner->GetOffset();
		const FBox DrawnBox(Origin - DrawnHalfExtents, Origin + DrawnHalfExtents);
		const FBox SampledBox(Origin - SampledHalfExtents, Origin + SampledHalfExtents);
		if (!DrawnBox.Min.Equals(SampledBox.Min, UE_KINDA_SMALL_NUMBER) ||
			!DrawnBox.Max.Equals(SampledBox.Max, UE_KINDA_SMALL_NUMBER))
		{
			ADD_ERROR("Drawn Box and sampled Box must occupy the same world-space bounds.");
		}
	});
}

#endif //WITH_TESTS

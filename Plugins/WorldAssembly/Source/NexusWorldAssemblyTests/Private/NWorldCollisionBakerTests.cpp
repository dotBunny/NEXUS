// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldCollisionBaker.h"
#include "Developer/NTestUtils.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Macros/NTestMacros.h"
#include "Organ/NOrganComponent.h"
#include "Organ/NOrganVolume.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionBakerHarness
{
	/** @return The organ component on a freshly spawned organ volume in World. */
	static UNOrganComponent* SpawnVolumeOrgan(UWorld* World)
	{
		ANOrganVolume* Volume = World->SpawnActor<ANOrganVolume>(ANOrganVolume::StaticClass(),
			FVector::ZeroVector, FRotator::ZeroRotator);
		if (Volume == nullptr) return nullptr;

		return Volume->FindComponentByClass<UNOrganComponent>();
	}
}

N_TEST_CRITICAL(FNWorldCollisionBakerTests_GetOrganBounds_VolumeOrganIsBounded,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionBaker::GetOrganBounds::VolumeOrganIsBounded",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionBakerHarness;

	FNTestUtils::WorldTest(EWorldType::Game, [](UWorld* World)
	{
		UNOrganComponent* Organ = SpawnVolumeOrgan(World);
		if (Organ == nullptr)
		{
			ADD_ERROR("Could not spawn an organ volume to measure.");
			return;
		}

		Organ->bUnbound = false;

		TArray<FBoxSphereBounds> Bounds;
		const FNWorldCollisionBaker::EOrganBoundsKind Kind = FNWorldCollisionBaker::GetOrganBounds(Organ, Bounds);

		CHECK_MESSAGE(TEXT("A bounded organ on a volume must report Bounded."),
			Kind == FNWorldCollisionBaker::EOrganBoundsKind::Bounded);
		CHECK_EQUALS("A bounded organ must contribute exactly its own volume's bounds.", Bounds.Num(), 1);
	});
}

N_TEST_CRITICAL(FNWorldCollisionBakerTests_GetOrganBounds_UnboundOrganIsUnbounded,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionBaker::GetOrganBounds::UnboundOrganIsUnbounded",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionBakerHarness;

	// The case that makes a cache cover the whole level. Reporting it as Bounded would cache only the organ volume's
	// own region for an organ that is entitled to place cells anywhere.
	FNTestUtils::WorldTest(EWorldType::Game, [](UWorld* World)
	{
		UNOrganComponent* Organ = SpawnVolumeOrgan(World);
		if (Organ == nullptr)
		{
			ADD_ERROR("Could not spawn an organ volume to measure.");
			return;
		}

		Organ->bUnbound = true;

		TArray<FBoxSphereBounds> Bounds;
		const FNWorldCollisionBaker::EOrganBoundsKind Kind = FNWorldCollisionBaker::GetOrganBounds(Organ, Bounds);

		CHECK_MESSAGE(TEXT("An unbound organ must report Unbounded even when it sits on a volume."),
			Kind == FNWorldCollisionBaker::EOrganBoundsKind::Unbounded);
		CHECK_MESSAGE(TEXT("An unbound organ must contribute no bounds, which every gather reads as the whole level."),
			Bounds.IsEmpty());
	});
}

N_TEST_CRITICAL(FNWorldCollisionBakerTests_GetOrganBounds_NonVolumeBoundedOrganContributesNothing,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionBaker::GetOrganBounds::NonVolumeBoundedOrganContributesNothing",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The distinction the tri-state exists for. A bounded organ with no volume is skipped by
	// FNAssemblyOperationContext::LockAndPreprocess, so it must report None rather than an empty bounds array —
	// which would be read as "the whole level" and make a cached run see geometry a fresh run never gathered.
	FNTestUtils::WorldTest(EWorldType::Game, [](UWorld* World)
	{
		AActor* PlainActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (PlainActor == nullptr)
		{
			ADD_ERROR("Could not spawn an actor to host the organ.");
			return;
		}

		UNOrganComponent* Organ = NewObject<UNOrganComponent>(PlainActor);
		Organ->RegisterComponent();
		Organ->bUnbound = false;

		TArray<FBoxSphereBounds> Bounds;
		const FNWorldCollisionBaker::EOrganBoundsKind Kind = FNWorldCollisionBaker::GetOrganBounds(Organ, Bounds);

		CHECK_MESSAGE(TEXT("A bounded organ with no volume must report None, not Unbounded."),
			Kind == FNWorldCollisionBaker::EOrganBoundsKind::None);
		CHECK_MESSAGE(TEXT("A None organ must contribute no bounds."), Bounds.IsEmpty());
	});
}

N_TEST_HIGH(FNWorldCollisionBakerTests_GetOrganBounds_NullOrganContributesNothing,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionBaker::GetOrganBounds::NullOrganContributesNothing",
	N_TEST_CONTEXT_ANYWHERE)
{
	TArray<FBoxSphereBounds> Bounds;
	Bounds.Add(FBoxSphereBounds(FVector::ZeroVector, FVector::OneVector, 1.0));

	const FNWorldCollisionBaker::EOrganBoundsKind Kind = FNWorldCollisionBaker::GetOrganBounds(nullptr, Bounds);

	CHECK_MESSAGE(TEXT("A null organ must report None."), Kind == FNWorldCollisionBaker::EOrganBoundsKind::None);
	CHECK_MESSAGE(TEXT("A null organ must clear whatever the caller passed in, never leave it to be read as bounds."),
		Bounds.IsEmpty());
}

N_TEST_HIGH(FNWorldCollisionBakerTests_BakeOrgan_NullWorldFails,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionBaker::BakeOrgan::NullWorldFails",
	N_TEST_CONTEXT_ANYWHERE)
{
	FNWorldCollisionBaker::FOrganResult Result;
	const bool bBaked = FNWorldCollisionBaker::BakeOrgan(nullptr, {}, FNWorldAssemblyWorldCollisionSettings(), Result);

	CHECK_FALSE_MESSAGE(TEXT("Baking a null world must fail rather than produce an empty-but-valid cache."), bBaked);
	CHECK_MESSAGE(TEXT("A failed bake must leave no fingerprint, so nothing can validate against it."),
		Result.Fingerprint == 0);
}

N_TEST_HIGH(FNWorldCollisionBakerTests_BakeOrgan_EmptyWorldProducesAValidFingerprint,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionBaker::BakeOrgan::EmptyWorldProducesAValidFingerprint",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A level with no collision geometry is a legitimate thing to cache, and must be distinguishable from never
	// having been baked — otherwise such an organ re-gathers an empty world on every single run.
	FNTestUtils::WorldTest(EWorldType::Game, [](UWorld* World)
	{
		FNWorldCollisionBaker::FOrganResult Result;
		const bool bBaked = FNWorldCollisionBaker::BakeOrgan(World, {}, FNWorldAssemblyWorldCollisionSettings(), Result);

		CHECK_MESSAGE(TEXT("Baking an empty world must succeed."), bBaked);
		CHECK_MESSAGE(TEXT("An empty bake must still carry a non-zero fingerprint."), Result.Fingerprint != 0);
		CHECK_EQUALS("An empty world yields no keys.", Result.Keys.Num(), 0);
		CHECK_EQUALS("Keys and meshes must stay parallel.", Result.Keys.Num(), Result.Meshes.Num());
	});
}

#endif //WITH_TESTS

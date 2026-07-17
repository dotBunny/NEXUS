// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NActorPool.h"
#include "NTestPooledActor.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(FNActorPoolTests_SpawnState_SpawnedTransformRotationCombines,
	"NEXUS::UnitTests::NActorPools::FNActorPool::SpawnState::SpawnedTransformRotationCombines",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that ApplySpawnState combines the requested InRotation with Settings.SpawnedTransform's
	// rotation as a quaternion product (InRotation * SpawnedTransform), rather than overwriting one
	// with the other or applying them in the opposite order.
	// Uses cross-axis rotations (pitch on SpawnedTransform, yaw on the spawn argument) so the
	// quaternion product is non-commutative — an accidental order swap will fail the assertion.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		const FRotator SpawnedTransformRotation(30.0f, 0.0f, 0.0f);
		const FRotator SpawnArgRotation(0.0f, 45.0f, 0.0f);

		FNActorPoolSettings Settings;
		Settings.MinimumActorCount = 1;
		Settings.MaximumActorCount = 1;
		Settings.Strategy = ENActorPoolStrategy::Fixed;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction);
		Settings.SpawnedTransform = FTransform(SpawnedTransformRotation, FVector::ZeroVector, FVector::OneVector);

		FNActorPool Pool = FNActorPool(World, ANTestPooledActor::StaticClass(), Settings);
		Pool.Fill();

		AActor* Actor = Pool.Spawn(FVector::ZeroVector, SpawnArgRotation);
		if (Actor == nullptr)
		{
			ADD_ERROR("Spawn() returned nullptr unexpectedly.");
			Pool.Clear();
			return;
		}

		const FQuat Expected = SpawnArgRotation.Quaternion() * SpawnedTransformRotation.Quaternion();
		const FQuat Swapped = SpawnedTransformRotation.Quaternion() * SpawnArgRotation.Quaternion();
		const FQuat Actual = Actor->GetActorQuat();

		// Self-check: if the chosen test rotations were commutative the test would silently
		// pass an order-swap regression. Catch that before it can mask a real bug.
		if (Expected.Equals(Swapped, 0.001))
		{
			ADD_ERROR("Test rotations are commutative; choose non-aligned axes so the assertion is order-sensitive.");
		}

		if (!Actual.Equals(Expected, 0.001))
		{
			ADD_ERROR(FString::Printf(TEXT("Spawned rotation %s does not match expected combined rotation %s (swapped order would be %s)."),
				*Actual.Rotator().ToString(), *Expected.Rotator().ToString(), *Swapped.Rotator().ToString()));
		}

		Pool.Clear();
	});
}

N_TEST_MEDIUM(FNActorPoolTests_SpawnState_SpawnedTransformLocationCombines,
	"NEXUS::UnitTests::NActorPools::FNActorPool::SpawnState::SpawnedTransformLocationCombines",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that ApplySpawnState adds Settings.SpawnedTransform's location to the InPosition
	// argument — same combine-not-overwrite contract as the rotation path.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		const FVector SpawnedTransformOffset(10.0f, 20.0f, 30.0f);
		const FVector SpawnArgPosition(100.0f, 0.0f, 0.0f);

		FNActorPoolSettings Settings;
		Settings.MinimumActorCount = 1;
		Settings.MaximumActorCount = 1;
		Settings.Strategy = ENActorPoolStrategy::Fixed;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction);
		Settings.SpawnedTransform = FTransform(FRotator::ZeroRotator, SpawnedTransformOffset, FVector::OneVector);

		FNActorPool Pool = FNActorPool(World, ANTestPooledActor::StaticClass(), Settings);
		Pool.Fill();

		AActor* Actor = Pool.Spawn(SpawnArgPosition, FRotator::ZeroRotator);
		if (Actor == nullptr)
		{
			ADD_ERROR("Spawn() returned nullptr unexpectedly.");
			Pool.Clear();
			return;
		}

		const FVector Expected = SpawnArgPosition + SpawnedTransformOffset;
		const FVector Actual = Actor->GetActorLocation();

		if (!Actual.Equals(Expected, 0.1))
		{
			ADD_ERROR(FString::Printf(TEXT("Spawned location %s does not match expected combined location %s."),
				*Actual.ToString(), *Expected.ToString()));
		}

		Pool.Clear();
	});
}

#endif //WITH_TESTS

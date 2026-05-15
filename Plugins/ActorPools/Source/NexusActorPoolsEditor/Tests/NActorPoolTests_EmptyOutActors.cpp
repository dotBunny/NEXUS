// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NActorPool.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(FNActorPoolTests_Strategy_FixedRecycleFirst_EmptyOutActors,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Strategy::FixedRecycleFirst::EmptyOutActors",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that Spawn() on a FixedRecycleFirst pool with both InActors and OutActors empty
	// returns nullptr instead of dereferencing OutActors[0]; covers the IsEmpty() guard in
	// FNActorPool::ApplyStrategy.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings Settings;
		Settings.MinimumActorCount = 0;
		Settings.MaximumActorCount = 3;
		Settings.Strategy = ENActorPoolStrategy::FixedRecycleFirst;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), Settings);
		// Intentionally no Fill() / no prior Spawn(); both InActors and OutActors are empty.

		AActor* Result = Pool.Spawn(FVector::ZeroVector, FRotator::ZeroRotator);
		if (Result != nullptr)
		{
			ADD_ERROR("Spawn() on an empty FixedRecycleFirst pool must return nullptr.");
		}
		CHECK_EQUALS("Available count must stay 0 after a failed Spawn().", Pool.GetAvailableCount(), 0)
		CHECK_EQUALS("Spawned count must stay 0 after a failed Spawn().", Pool.GetSpawnedCount(), 0)

		Pool.Clear();
	});
}

N_TEST_HIGH(FNActorPoolTests_Strategy_FixedRecycleLast_EmptyOutActors,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Strategy::FixedRecycleLast::EmptyOutActors",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that Spawn() on a FixedRecycleLast pool with both InActors and OutActors empty
	// returns nullptr instead of dereferencing OutActors.Last(); covers the IsEmpty() guard in
	// FNActorPool::ApplyStrategy.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings Settings;
		Settings.MinimumActorCount = 0;
		Settings.MaximumActorCount = 3;
		Settings.Strategy = ENActorPoolStrategy::FixedRecycleLast;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), Settings);
		// Intentionally no Fill() / no prior Spawn(); both InActors and OutActors are empty.

		AActor* Result = Pool.Spawn(FVector::ZeroVector, FRotator::ZeroRotator);
		if (Result != nullptr)
		{
			ADD_ERROR("Spawn() on an empty FixedRecycleLast pool must return nullptr.");
		}
		CHECK_EQUALS("Available count must stay 0 after a failed Spawn().", Pool.GetAvailableCount(), 0)
		CHECK_EQUALS("Spawned count must stay 0 after a failed Spawn().", Pool.GetSpawnedCount(), 0)

		Pool.Clear();
	});
}

N_TEST_MEDIUM(FNActorPoolTests_Strategy_FixedRecycleFirst_GetEmptyOutActors,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Strategy::FixedRecycleFirst::GetEmptyOutActors",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that Get() shares the same IsEmpty() guard as Spawn() on FixedRecycleFirst; both
	// flow through ApplyStrategy, so an empty pool must return nullptr from Get() too.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings Settings;
		Settings.MinimumActorCount = 0;
		Settings.MaximumActorCount = 3;
		Settings.Strategy = ENActorPoolStrategy::FixedRecycleFirst;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), Settings);

		AActor* Result = Pool.Get();
		if (Result != nullptr)
		{
			ADD_ERROR("Get() on an empty FixedRecycleFirst pool must return nullptr.");
		}

		Pool.Clear();
	});
}

#endif //WITH_TESTS

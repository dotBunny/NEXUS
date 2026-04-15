// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NActorPool.h"
#include "NPooledActor.h"
#include "Developer/NDebugActor.h"
#include "Developer/NDeveloperUtils.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST(FNActorPoolTests_LeakCheck_DontForceDestroy,
	"NEXUS::UnitTests::NActorPools::FNActorPool::LeakCheck::DontForceDestroy",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that clearing without force-destroy does not leak the pre-created pool actors.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		const int32 PrePoolObjects = FNDeveloperUtils::GetCurrentObjectCount();
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);
		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		const int32 PostPoolObjects = FNDeveloperUtils::GetCurrentObjectCount();
		CHECK_EQUALS("Check that no additional objects were created when filling the pool.", PostPoolObjects, PrePoolObjects+Pool.GetSettings().MinimumActorCount);

		const int32 PooledActorCount = Pool.GetInCount();
		CHECK_EQUALS("Check that the expected number of Actors were made.", PooledActorCount, Pool.GetSettings().MinimumActorCount);

		Pool.Clear(false);
	});
}

N_TEST(FNActorPoolTests_LeakCheck_ForceDestroy,
	"NEXUS::UnitTests::NActorPools::FNActorPool::LeakCheck::ForceDestroy",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that clearing with force-destroy and a subsequent GC pass returns the object count to baseline.
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		const int32 PrePoolObjects = FNDeveloperUtils::GetCurrentObjectCount();

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass());
		Pool.Fill();
		Pool.Clear(true);

		// Force destroy tells the actors to be destroyed; it doesn't mean we actually will have them cleaned up unless we do a GC pass
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		const int32 PostClearObjects = FNDeveloperUtils::GetCurrentObjectCount();

		CHECK_EQUALS("Check the number of objects matches the previous count when forcibly destroying the pool.", PostClearObjects, PrePoolObjects);
	}, true);
}

N_TEST(FNActorPoolTests_Get_BasicRetrieval,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Get::BasicRetrieval",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that Get() moves exactly one actor from In to Out and returns a valid pointer.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 3;
		ActorPoolSettings.MaximumActorCount = 3;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::Fixed;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		CHECK_EQUALS("Pool should have 3 actors in before Get().", Pool.GetInCount(), 3)
		CHECK_EQUALS("Pool should have 0 actors out before Get().", Pool.GetOutCount(), 0)

		AActor* Actor = Pool.Get();
		if (Actor == nullptr)
		{
			ADD_ERROR("Get() returned nullptr unexpectedly.");
			Pool.Clear();
			return;
		}

		CHECK_EQUALS("Pool should have 2 actors in after Get().", Pool.GetInCount(), 2)
		CHECK_EQUALS("Pool should have 1 actor out after Get().", Pool.GetOutCount(), 1)

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_Get_DoesNotTriggerSpawnEvents,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Get::DoesNotTriggerSpawnEvents",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that Get() does not invoke OnSpawnedFromActorPool, unlike Spawn().
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 1;
		ActorPoolSettings.MaximumActorCount = 1;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::Fixed;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, ANPooledActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		ANPooledActor* Actor = Cast<ANPooledActor>(Pool.Get());
		if (Actor == nullptr)
		{
			ADD_ERROR("Get() returned nullptr unexpectedly.");
			Pool.Clear();
			return;
		}

		// After creation and Fill(), the actor should be Disabled (returned to pool).
		// Get() must not advance it to Enabled — only Spawn() should do that.
		if (Actor->GetCurrentActorOperationalState() == ENActorOperationalState::Enabled)
		{
			ADD_ERROR("Get() should not invoke OnSpawnedFromActorPool; actor operational state must not be Enabled.");
		}

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_Return_MovesActorBackToIn,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Return::MovesActorBackToIn",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that Return() moves a spawned actor from Out back to In and returns true.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 3;
		ActorPoolSettings.MaximumActorCount = 3;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::Fixed;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		AActor* Actor = Pool.Spawn(FVector::Zero(), FRotator::ZeroRotator);
		if (Actor == nullptr)
		{
			ADD_ERROR("Spawn() returned nullptr unexpectedly.");
			Pool.Clear();
			return;
		}

		CHECK_EQUALS("Pool should have 2 in after Spawn().", Pool.GetInCount(), 2)
		CHECK_EQUALS("Pool should have 1 out after Spawn().", Pool.GetOutCount(), 1)

		const bool bResult = Pool.Return(Actor);
		CHECK_EQUALS("Return() should return true.", bResult, true)
		CHECK_EQUALS("Pool should have 3 in after Return().", Pool.GetInCount(), 3)
		CHECK_EQUALS("Pool should have 0 out after Return().", Pool.GetOutCount(), 0)

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_Return_NullArgument,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Return::NullArgument",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that Return(nullptr) does not crash and returns false gracefully.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		AddExpectedMessage(TEXT("Attempted to return a NULL reference to a FNActorPool"), ELogVerbosity::Type::Warning);
		const bool bResult = Pool.Return(nullptr);
		CHECK_EQUALS("Return(nullptr) should return false.", bResult, false)

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_SpawnState_ActorActivated,
	"NEXUS::UnitTests::NActorPools::FNActorPool::SpawnState::ActorActivated",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that Spawn() makes the actor visible and enables collision.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 1;
		ActorPoolSettings.MaximumActorCount = 1;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::Fixed;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		AActor* Actor = Pool.Spawn(FVector::Zero(), FRotator::ZeroRotator);
		if (Actor == nullptr)
		{
			ADD_ERROR("Spawn() returned nullptr unexpectedly.");
			Pool.Clear();
			return;
		}

		CHECK_MESSAGE(TEXT("Spawned actor should not be hidden."), !Actor->IsHidden())
		CHECK_MESSAGE(TEXT("Spawned actor should have collision enabled."), Actor->GetActorEnableCollision())

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_SpawnState_ActorAtRequestedLocation,
	"NEXUS::UnitTests::NActorPools::FNActorPool::SpawnState::ActorAtRequestedLocation",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that Spawn() places the actor at the requested world position.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 1;
		ActorPoolSettings.MaximumActorCount = 1;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::Fixed;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, ANDebugActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		const FVector SpawnPos(100.0f, 200.0f, 300.0f);
		AActor* Actor = Pool.Spawn(SpawnPos, FRotator::ZeroRotator);
		if (Actor == nullptr)
		{
			ADD_ERROR("Spawn() returned nullptr unexpectedly.");
			Pool.Clear();
			return;
		}

		const FVector ActualPos = Actor->GetActorLocation();
		if (!ActualPos.Equals(SpawnPos, 0.1f))
		{
			ADD_ERROR(FString::Printf(TEXT("Actor location %s does not match requested spawn position %s."),
				*ActualPos.ToString(), *SpawnPos.ToString()));
		}

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_ReturnState_ActorDeactivated,
	"NEXUS::UnitTests::NActorPools::FNActorPool::ReturnState::ActorDeactivated",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that returning an actor hides it, disables collision, and disables tick.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 1;
		ActorPoolSettings.MaximumActorCount = 1;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::Fixed;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		AActor* Actor = Pool.Spawn(FVector::Zero(), FRotator::ZeroRotator);
		if (Actor == nullptr)
		{
			ADD_ERROR("Spawn() returned nullptr unexpectedly.");
			Pool.Clear();
			return;
		}

		Pool.Return(Actor);

		CHECK_MESSAGE(TEXT("Returned actor should be hidden."), Actor->IsHidden())
		CHECK_MESSAGE(TEXT("Returned actor should have collision disabled."), !Actor->GetActorEnableCollision())
		CHECK_MESSAGE(TEXT("Returned actor should have tick disabled."), !Actor->IsActorTickEnabled())

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_Clear_WithOutActors,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Clear::WithOutActors",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that Clear() destroys both in-pool and currently-spawned-out actors, with no leaks after GC.
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		const int32 PrePoolObjects = FNDeveloperUtils::GetCurrentObjectCount();

		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 4;
		ActorPoolSettings.MaximumActorCount = 4;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::Fixed;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		// Spawn 2 and leave them out
		Pool.Spawn(FVector::Zero(), FRotator::ZeroRotator);
		Pool.Spawn(FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator);

		CHECK_EQUALS("Pool should have 2 actors in.", Pool.GetInCount(), 2)
		CHECK_EQUALS("Pool should have 2 actors out.", Pool.GetOutCount(), 2)

		Pool.Clear(true);

		CHECK_EQUALS("Pool should have 0 in after Clear().", Pool.GetInCount(), 0)
		CHECK_EQUALS("Pool should have 0 out after Clear().", Pool.GetOutCount(), 0)

		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		const int32 PostClearObjects = FNDeveloperUtils::GetCurrentObjectCount();
		CHECK_EQUALS("No actors should remain after Clear() and GC.", PostClearObjects, PrePoolObjects)
	}, true);
}

N_TEST(FNActorPoolTests_Fill_RespectsMinimum,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Fill::RespectsMinimum",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that a second Fill() does not add actors when the minimum is already satisfied.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 3;
		ActorPoolSettings.MaximumActorCount = 20;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::Create;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		CHECK_EQUALS("Fill() should create MinimumActorCount actors.", Pool.GetInCount(), 3)

		Pool.Fill();

		CHECK_EQUALS("A second Fill() should not add actors when the minimum is already met.", Pool.GetInCount(), 3)

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_Prewarm_AddsAboveMinimum,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Prewarm::AddsAboveMinimum",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that Prewarm() adds actors unconditionally on top of whatever is already in the pool.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 3;
		ActorPoolSettings.MaximumActorCount = 20;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::Create;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		CHECK_EQUALS("Fill() should create MinimumActorCount actors.", Pool.GetInCount(), 3)

		Pool.Prewarm(5);

		CHECK_EQUALS("Prewarm() should add on top of the existing pool count.", Pool.GetInCount(), 8)

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_UpdateSettings_ChangesMinimum,
	"NEXUS::UnitTests::NActorPools::FNActorPool::UpdateSettings::ChangesMinimum",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that UpdateSettings() persists the new MinimumActorCount and a subsequent Fill() respects it.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 2;
		ActorPoolSettings.MaximumActorCount = 10;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::Create;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		CHECK_EQUALS("Initial fill should create MinimumActorCount actors.", Pool.GetInCount(), 2)

		FNActorPoolSettings NewSettings = ActorPoolSettings;
		NewSettings.MinimumActorCount = 5;
		Pool.UpdateSettings(NewSettings);

		CHECK_EQUALS("Settings should reflect the new MinimumActorCount.", Pool.GetSettings().MinimumActorCount, 5)

		Pool.Fill();

		CHECK_EQUALS("Fill() after UpdateSettings() should bring the pool up to the new minimum.", Pool.GetInCount(), 5)

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_Strategy_Create,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Strategy::Create",
	N_TEST_CONTEXT_EDITOR)
{
	// Tests the Create fill strategy of an actor pool; verifies that it will make above the maximum count if requested.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 0;
		ActorPoolSettings.MaximumActorCount = 5;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::Create;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		// Create some actors
		TArray<AActor*> Actors;
		Actors.Reserve(10);
		for (int32 i = 0; i < 10; i++)
		{
			Actors.Add(Pool.Spawn(FVector::Zero() + FVector(100*i, 0,0), FRotator::ZeroRotator));
		}

		CHECK_EQUALS("Check the pools OutActors.", Pool.GetOutCount(), 10)
		CHECK_EQUALS("Check the pools InActors.",Pool.GetInCount(), 0)

		// Check the created actors for unique pointers
		for (int32 i = 0; i < 10; i++)
		{
			for (int32 j = 0; j < 10; j++)
			{
				if (i == j) continue;
				if (Actors[i] == Actors[j])
				{
					ADD_ERROR("The created actors were not unique, found the same pointers.");
				}
			}
		}

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_Strategy_CreateLimited,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Strategy::CreateLimited",
	N_TEST_CONTEXT_EDITOR)
{
	// Tests the CreateLimited fill strategy of an actor pool; verifies that it will stop when the maximum count is hit.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 0;
		ActorPoolSettings.MaximumActorCount = 2;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::CreateLimited;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		// Create some actors
		TArray<AActor*> Actors;
		Actors.Reserve(10);
		for (int32 i = 0; i < 10; i++)
		{
			Actors.Add(Pool.Spawn(FVector::Zero() + FVector(100*i, 0,0), FRotator::ZeroRotator));
		}

		CHECK_EQUALS("Check the pools OutActors.", Pool.GetOutCount(), 2)
		CHECK_EQUALS("Check the pools InActors.",Pool.GetInCount(), 0)

		// Check the created actors for unique pointers
		for (int32 i = 0; i < 2; i++)
		{
			if (Actors[i] == nullptr)
			{
				ADD_ERROR("An invalid Actor pointer was returned where expected.")
			}
		}
		for (int32 i = 2; i < 10; i++)
		{
			if (Actors[i] != nullptr)
			{
				ADD_ERROR("We expected all null pointers above the limited spawns.")
			}
		}

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_Strategy_CreateRecycleFirst,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Strategy::CreateRecycleFirst",
	N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 5;
		ActorPoolSettings.MaximumActorCount = 5;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::CreateRecycleFirst;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);


		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		// Create some actors
		TArray<AActor*> Actors;
		Actors.Reserve(5);
		for (int32 i = 0; i < 5; i++)
		{
			Actors.Add(Pool.Spawn(FVector::Zero() + FVector(100*i, 0,0), FRotator::ZeroRotator));
		}

		CHECK_EQUALS("Check the pools OutActors.", Pool.GetOutCount(), 5)
		CHECK_EQUALS("Check the pools InActors.",Pool.GetInCount(), 0)

		// Check that we recycle the first spawned actor
		AActor* FirstActor = Pool.Spawn(FVector::Zero() + FVector(100*10, 0,0), FRotator::ZeroRotator);
		if (Actors[0] != FirstActor)
		{
			ADD_ERROR("The first recycled actor was not the first out actor, this is unexpected.")
		}

		// Check that we recycle the second spawned actor
		AActor* SecondActor = Pool.Spawn(FVector::Zero() + FVector(100*11, 0,0), FRotator::ZeroRotator);
		if (Actors[1] != SecondActor)
		{
			ADD_ERROR("The second recycled actor was not the second out actor, this is unexpected.")
		}

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_Strategy_CreateRecycleFirst_RecyclesWhenAtMax,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Strategy::CreateRecycleFirst::RecyclesWhenAtMax",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies CreateRecycleFirst creates new actors up to the maximum, then recycles the oldest out actor.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 0;
		ActorPoolSettings.MaximumActorCount = 3;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::CreateRecycleFirst;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);

		// Spawn up to the maximum — all should create new unique actors
		TArray<AActor*> Actors;
		Actors.Reserve(3);
		for (int32 i = 0; i < 3; i++)
		{
			Actors.Add(Pool.Spawn(FVector(100.0f * i, 0.0f, 0.0f), FRotator::ZeroRotator));
		}

		CHECK_EQUALS("Pool should have 3 out after reaching max.", Pool.GetOutCount(), 3)
		CHECK_EQUALS("Pool should have 0 in when at max.", Pool.GetInCount(), 0)

		// 4th spawn should recycle the first out actor
		AActor* Recycled = Pool.Spawn(FVector(300.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
		if (Recycled != Actors[0])
		{
			ADD_ERROR("CreateRecycleFirst should recycle the oldest out actor when at maximum capacity.");
		}

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_Strategy_CreateRecycleLast,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Strategy::CreateRecycleLast",
	N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 5;
		ActorPoolSettings.MaximumActorCount = 5;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::CreateRecycleLast;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);


		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		// Create some actors
		TArray<AActor*> Actors;
		Actors.Reserve(5);
		for (int32 i = 0; i < 5; i++)
		{
			Actors.Add(Pool.Spawn(FVector::Zero() + FVector(100*i, 0,0), FRotator::ZeroRotator));
		}

		CHECK_EQUALS("Check the pools OutActors.", Pool.GetOutCount(), 5)
		CHECK_EQUALS("Check the pools InActors.",Pool.GetInCount(), 0)

		// Test that it recycles the last actor
		AActor* FirstLastActor = Pool.Spawn(FVector::Zero() + FVector(100*10, 0,0), FRotator::ZeroRotator);
		if (FirstLastActor == nullptr)
		{
			ADD_ERROR("The first recycled actor was null, this is unexpected.")
		}
		if (FirstLastActor != Actors[4])
		{
			ADD_ERROR("The first recycled actor was not the last actor, this is unexpected.")
		}

		// Should be the same because we just reuse the last
		AActor* LastActorAgain = Pool.Spawn(FVector::Zero() + FVector(100*11, 0,0), FRotator::ZeroRotator);
		if (FirstLastActor != LastActorAgain)
		{
			ADD_ERROR("The second recycled actor was not the same as the first, this is unexpected.")
		}

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_Strategy_CreateRecycleLast_RecyclesWhenAtMax,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Strategy::CreateRecycleLast::RecyclesWhenAtMax",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies CreateRecycleLast creates new actors up to the maximum, then recycles the newest out actor.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 0;
		ActorPoolSettings.MaximumActorCount = 3;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::CreateRecycleLast;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);

		// Spawn up to the maximum — all should create new unique actors
		TArray<AActor*> Actors;
		Actors.Reserve(3);
		for (int32 i = 0; i < 3; i++)
		{
			Actors.Add(Pool.Spawn(FVector(100.0f * i, 0.0f, 0.0f), FRotator::ZeroRotator));
		}

		CHECK_EQUALS("Pool should have 3 out after reaching max.", Pool.GetOutCount(), 3)
		CHECK_EQUALS("Pool should have 0 in when at max.", Pool.GetInCount(), 0)

		// 4th spawn should recycle the last out actor
		AActor* Recycled = Pool.Spawn(FVector(300.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
		if (Recycled != Actors[2])
		{
			ADD_ERROR("CreateRecycleLast should recycle the newest out actor when at maximum capacity.");
		}

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_Strategy_Fixed,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Strategy::Fixed",
	N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 3;
		ActorPoolSettings.MaximumActorCount = 3;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::Fixed;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);


		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		// Create some actors
		TArray<AActor*> Actors;
		Actors.Reserve(3);
		for (int32 i = 0; i < 3; i++)
		{
			Actors.Add(Pool.Spawn(FVector::Zero() + FVector(100*i, 0,0), FRotator::ZeroRotator));
		}

		CHECK_EQUALS("Check the pools OutActors.", Pool.GetOutCount(), 3)
		CHECK_EQUALS("Check the pools InActors.",Pool.GetInCount(), 0)

		AActor* BlockedActor = Pool.Spawn(FVector::Zero() + FVector(100*10, 0,0), FRotator::ZeroRotator);
		if (BlockedActor != nullptr)
		{
			ADD_ERROR("Expected a nullptr from attempting to pull from a expended pool.")
		}

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_Strategy_FixedRecycleFirst,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Strategy::FixedRecycleFirst",
	N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 2;
		ActorPoolSettings.MaximumActorCount = 3;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::FixedRecycleFirst;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);


		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		// Create some actors
		TArray<AActor*> Actors;
		Actors.Reserve(2);
		for (int32 i = 0; i < 2; i++)
		{
			Actors.Add(Pool.Spawn(FVector::Zero() + FVector(100*i, 0,0), FRotator::ZeroRotator));
		}

		CHECK_EQUALS("Check the pools OutActors.", Pool.GetOutCount(), 2)
		CHECK_EQUALS("Check the pools InActors.",Pool.GetInCount(), 0)

		// Check that we recycle the first spawned actor
		AActor* FirstActor = Pool.Spawn(FVector::Zero() + FVector(100*10, 0,0), FRotator::ZeroRotator);
		if (Actors[0] != FirstActor)
		{
			ADD_ERROR("The first recycled actor was not the first out actor, this is unexpected.")
		}

		// Check that we recycle the second spawned actor
		AActor* SecondActor = Pool.Spawn(FVector::Zero() + FVector(100*11, 0,0), FRotator::ZeroRotator);
		if (Actors[1] != SecondActor)
		{
			ADD_ERROR("The second recycled actor was not the second out actor, this is unexpected.")
		}

		Pool.Clear();
	});
}

N_TEST(FNActorPoolTests_Strategy_FixedRecycleLast,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Strategy::FixedRecycleLast",
	N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 3;
		ActorPoolSettings.MaximumActorCount = 3;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::FixedRecycleLast;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);


		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		// Create some actors
		TArray<AActor*> Actors;
		Actors.Reserve(3);
		for (int32 i = 0; i < 3; i++)
		{
			Actors.Add(Pool.Spawn(FVector::Zero() + FVector(100*i, 0,0), FRotator::ZeroRotator));
		}

		CHECK_EQUALS("Check the pools OutActors.", Pool.GetOutCount(), 3)
		CHECK_EQUALS("Check the pools InActors.",Pool.GetInCount(), 0)


		// Test that it recycles the last actor
		AActor* FirstLastActor = Pool.Spawn(FVector::Zero() + FVector(100*10, 0,0), FRotator::ZeroRotator);
		if (FirstLastActor == nullptr)
		{
			ADD_ERROR("The first recycled actor was null, this is unexpected.")
		}
		if (FirstLastActor != Actors[2])
		{
			ADD_ERROR("The first recycled actor was not the last actor, this is unexpected.")
		}

		// Should be the same because we just reuse the last
		AActor* LastActorAgain = Pool.Spawn(FVector::Zero() + FVector(100*11, 0,0), FRotator::ZeroRotator);
		if (FirstLastActor != LastActorAgain)
		{
			ADD_ERROR("The second recycled actor was not the same as the first, this is unexpected.")
		}

		Pool.Clear();
	});
}

#endif //WITH_TESTS

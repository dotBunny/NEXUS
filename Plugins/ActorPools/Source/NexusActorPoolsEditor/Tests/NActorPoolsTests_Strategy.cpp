// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NActorPool.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST(FNActorPoolTests_Strategy_Create, "NEXUS::UnitTests::NActorPools::Strategy::Create", N_TEST_CONTEXT_EDITOR)
{
	// Tests the Create fill strategy of an actor pool; verifies that it will make above the maximum count if requested.
	FNTestUtils::WorldTest(EWorldType::Editor, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 0;
		ActorPoolSettings.MaximumActorCount = 5;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::Create;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorageLocation | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);
	
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

N_TEST(FNActorPoolTests_Strategy_CreateLimited, "NEXUS::UnitTests::NActorPools::Strategy::CreateLimited", N_TEST_CONTEXT_EDITOR)
{
	// Tests the CreateLimited fill strategy of an actor pool; verifies that it will stop when the maximum count is hit.
	FNTestUtils::WorldTest(EWorldType::Editor, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 0;
		ActorPoolSettings.MaximumActorCount = 2;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::CreateLimited;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorageLocation | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);
	
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

N_TEST(FNActorPoolTests_Strategy_CreateRecycleFirst, "NEXUS::UnitTests::NActorPools::Strategy::CreateRecycleFirst", N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 5;
		ActorPoolSettings.MaximumActorCount = 5;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::CreateRecycleFirst;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorageLocation | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);
	
	
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

N_TEST(FNActorPoolTests_Strategy_CreateRecycleLast, "NEXUS::UnitTests::NActorPools::Strategy::CreateRecycleLast", N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTest(EWorldType::Editor, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 5;
		ActorPoolSettings.MaximumActorCount = 5;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::CreateRecycleLast;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorageLocation | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);
	
	
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

N_TEST(FNActorPoolTests_Strategy_Fixed, "NEXUS::UnitTests::NActorPools::Strategy::Fixed", N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTest(EWorldType::Editor, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 3;
		ActorPoolSettings.MaximumActorCount = 3;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::Fixed;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorageLocation | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);
	
	
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

N_TEST(FNActorPoolTests_Strategy_FixedRecycleFirst, "NEXUS::UnitTests::NActorPools::Strategy::FixedRecycleFirst", N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTest(EWorldType::Editor, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 2;
		ActorPoolSettings.MaximumActorCount = 3;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::FixedRecycleFirst;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorageLocation | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);
	
	
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

N_TEST(FNActorPoolTests_Strategy_FixedRecycleLast, "NEXUS::UnitTests::NActorPools::Strategy::FixedRecycleLast", N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTest(EWorldType::Editor, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 3;
		ActorPoolSettings.MaximumActorCount = 3;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::FixedRecycleLast;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorageLocation | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);
	
	
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
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NActorPool.h"
#include "NActorPoolSubsystem.h"
#include "Developer/NDebugActor.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_CRITICAL(UNActorPoolSubsystemTests_CreateActorPool_CreatesPool,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::CreateActorPool::CreatesPool",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that CreateActorPool returns true and HasActorPool reflects the newly created pool.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		FNActorPoolSettings Settings;
		Settings.MinimumActorCount = 2;
		Settings.MaximumActorCount = 5;

		const bool bCreated = Subsystem->CreateActorPool(AActor::StaticClass(), Settings);
		CHECK_MESSAGE(TEXT("CreateActorPool should return true for a new actor class."), bCreated)
		CHECK_MESSAGE(TEXT("HasActorPool should return true after CreateActorPool."), Subsystem->HasActorPool(AActor::StaticClass()))
	});
}

N_TEST_CRITICAL(UNActorPoolSubsystemTests_CreateActorPool_NoDuplicate,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::CreateActorPool::NoDuplicate",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that creating a pool for the same class a second time is idempotent and returns false.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		FNActorPoolSettings Settings;
		Subsystem->CreateActorPool(AActor::StaticClass(), Settings);

		const bool bSecond = Subsystem->CreateActorPool(AActor::StaticClass(), Settings);
		CHECK_FALSE_MESSAGE(TEXT("CreateActorPool should return false when the pool already exists."), bSecond)
	});
}

N_TEST_HIGH(UNActorPoolSubsystemTests_HasActorPool_FalseForUnknownClass,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::HasActorPool::FalseForUnknownClass",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that HasActorPool returns false for a class that has never had a pool created.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		CHECK_FALSE_MESSAGE(TEXT("HasActorPool should return false for a class with no registered pool."),
			Subsystem->HasActorPool(ANDebugActor::StaticClass()))
	});
}

N_TEST_CRITICAL(UNActorPoolSubsystemTests_GetActor_LazyCreatesPool,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::GetActor::LazyCreatesPool",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that GetActor lazily creates a pool on first access and HasActorPool reflects this.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		CHECK_FALSE_MESSAGE(TEXT("HasActorPool should be false before GetActor is called."),
			Subsystem->HasActorPool(ANDebugActor::StaticClass()))

		Subsystem->GetActor<AActor>(ANDebugActor::StaticClass());

		CHECK_MESSAGE(TEXT("HasActorPool should be true after GetActor lazily creates the pool."),
			Subsystem->HasActorPool(ANDebugActor::StaticClass()))
	});
}

N_TEST_CRITICAL(UNActorPoolSubsystemTests_GetActor_ReturnsValidActor,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::GetActor::ReturnsValidActor",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that GetActor returns a live, in-world actor of the requested class and that the
	// pool's spawned-count reflects that the actor was sourced from the pool.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		AActor* Actor = Subsystem->GetActor<AActor>(ANDebugActor::StaticClass());
		if (Actor == nullptr)
		{
			ADD_ERROR("GetActor returned nullptr unexpectedly.");
			return;
		}

		if (!Actor->IsA(ANDebugActor::StaticClass()))
		{
			ADD_ERROR(FString::Printf(TEXT("GetActor returned an actor of class %s, expected ANDebugActor."),
				*Actor->GetClass()->GetName()));
			return;
		}

		if (Actor->GetWorld() != World)
		{
			ADD_ERROR("GetActor returned an actor that is not in the test world.");
			return;
		}

		FNActorPool* Pool = Subsystem->GetActorPool(ANDebugActor::StaticClass());
		if (Pool == nullptr)
		{
			ADD_ERROR("Pool for ANDebugActor was not created by GetActor.");
			return;
		}

		CHECK_EQUALS("Pool should record the GetActor result as a spawned actor.", Pool->GetSpawnedCount(), 1)
	});
}

N_TEST_CRITICAL(UNActorPoolSubsystemTests_SpawnActor_LazyCreatesPool,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::SpawnActor::LazyCreatesPool",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that SpawnActor lazily creates a pool on first access and HasActorPool reflects this.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		CHECK_FALSE_MESSAGE(TEXT("HasActorPool should be false before SpawnActor is called."),
			Subsystem->HasActorPool(ANDebugActor::StaticClass()))

		Subsystem->SpawnActor<AActor>(ANDebugActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);

		CHECK_MESSAGE(TEXT("HasActorPool should be true after SpawnActor lazily creates the pool."),
			Subsystem->HasActorPool(ANDebugActor::StaticClass()))
	});
}

N_TEST_HIGH(UNActorPoolSubsystemTests_SpawnActor_PlacesActorAtPosition,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::SpawnActor::PlacesActorAtPosition",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that SpawnActor places the returned actor at the requested world position.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		const FVector SpawnPos(100.0f, 200.0f, 300.0f);
		AActor* Actor = Subsystem->SpawnActor<AActor>(ANDebugActor::StaticClass(), SpawnPos, FRotator::ZeroRotator);
		if (Actor == nullptr)
		{
			ADD_ERROR("SpawnActor returned nullptr unexpectedly.");
			return;
		}

		const FVector ActualPos = Actor->GetActorLocation();
		if (!ActualPos.Equals(SpawnPos, 0.1f))
		{
			ADD_ERROR(FString::Printf(TEXT("Actor location %s does not match requested spawn position %s."),
				*ActualPos.ToString(), *SpawnPos.ToString()));
		}
	});
}

N_TEST_CRITICAL(UNActorPoolSubsystemTests_ReturnActor_ReturnsTrueForKnownPool,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::ReturnActor::ReturnsTrueForKnownPool",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that ReturnActor returns true AND the pool's available/spawned counts reflect the
	// return. A bool-only assertion would pass even if the implementation lied about success.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		FNActorPoolSettings Settings;
		Settings.MinimumActorCount = 3;
		Settings.MaximumActorCount = 3;
		Settings.Strategy = ENActorPoolStrategy::Fixed;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);
		Subsystem->CreateActorPool(AActor::StaticClass(), Settings);

		FNActorPool* Pool = Subsystem->GetActorPool(AActor::StaticClass());
		if (Pool == nullptr)
		{
			ADD_ERROR("Pool was not created.");
			return;
		}

		AActor* Actor = Subsystem->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Actor == nullptr)
		{
			ADD_ERROR("SpawnActor returned nullptr unexpectedly.");
			return;
		}

		const int32 SpawnedBefore = Pool->GetSpawnedCount();
		const int32 AvailableBefore = Pool->GetAvailableCount();
		CHECK_EQUALS("Pool should report exactly 1 spawned actor before return.", SpawnedBefore, 1)

		const bool bReturned = Subsystem->ReturnActor(Actor);
		CHECK_MESSAGE(TEXT("ReturnActor should return true for an actor from a known pool."), bReturned)
		CHECK_EQUALS("Pool's spawned count should drop by 1 after return.", Pool->GetSpawnedCount(), SpawnedBefore - 1)
		CHECK_EQUALS("Pool's available count should rise by 1 after return.", Pool->GetAvailableCount(), AvailableBefore + 1)
	});
}

N_TEST_HIGH(UNActorPoolSubsystemTests_GetActorPool_ReturnsCreatedPool,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::GetActorPool::ReturnsCreatedPool",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that GetActorPool returns a valid pointer with the correct template class.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		FNActorPoolSettings Settings;
		Subsystem->CreateActorPool(AActor::StaticClass(), Settings);

		FNActorPool* Pool = Subsystem->GetActorPool(AActor::StaticClass());
		if (Pool == nullptr)
		{
			ADD_ERROR("GetActorPool returned nullptr for a pool that was just created.");
			return;
		}

		if (Pool->GetTemplate() != AActor::StaticClass())
		{
			ADD_ERROR("GetActorPool returned a pool with an unexpected template class.");
		}
	});
}

N_TEST_HIGH(UNActorPoolSubsystemTests_GetAllPools_CountMatchesCreated,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::GetAllPools::CountMatchesCreated",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that GetAllPools returns the exact pool set we created - both count AND identity by
	// template class. A count-only assertion would pass even if the wrong pools were returned.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		const int32 CountBefore = Subsystem->GetAllPools().Num();

		FNActorPoolSettings Settings;
		Subsystem->CreateActorPool(AActor::StaticClass(), Settings);
		Subsystem->CreateActorPool(ANDebugActor::StaticClass(), Settings);

		const TArray<FNActorPool*> AllPools = Subsystem->GetAllPools();
		CHECK_EQUALS("GetAllPools should contain exactly two newly created pools.", AllPools.Num(), CountBefore + 2)

		bool bFoundActor = false;
		bool bFoundDebugActor = false;
		for (FNActorPool* Pool : AllPools)
		{
			if (Pool == nullptr) continue;
			if (Pool->GetTemplate() == AActor::StaticClass()) bFoundActor = true;
			if (Pool->GetTemplate() == ANDebugActor::StaticClass()) bFoundDebugActor = true;
		}
		CHECK_MESSAGE(TEXT("GetAllPools should contain a pool templated on AActor."), bFoundActor)
		CHECK_MESSAGE(TEXT("GetAllPools should contain a pool templated on ANDebugActor."), bFoundDebugActor)
	});
}

N_TEST_CRITICAL(UNActorPoolSubsystemTests_DefaultSettings_AddAndHas,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::DefaultSettings::AddAndHas",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that AddDefaultSettings registers the settings and HasDefaultSettings reflects them.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		CHECK_FALSE_MESSAGE(TEXT("HasDefaultSettings should be false before any default is registered."),
			Subsystem->HasDefaultSettings(ANDebugActor::StaticClass()))

		FNActorPoolSettings Settings;
		Settings.MinimumActorCount = 4;
		const bool bAdded = Subsystem->AddDefaultSettings(ANDebugActor::StaticClass(), Settings);
		CHECK_MESSAGE(TEXT("AddDefaultSettings should return true for a new class."), bAdded)
		CHECK_MESSAGE(TEXT("HasDefaultSettings should return true after adding."),
			Subsystem->HasDefaultSettings(ANDebugActor::StaticClass()))
	});
}

N_TEST_CRITICAL(UNActorPoolSubsystemTests_DefaultSettings_Remove,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::DefaultSettings::Remove",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that RemoveDefaultSettings returns true and HasDefaultSettings becomes false.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		FNActorPoolSettings Settings;
		Subsystem->AddDefaultSettings(ANDebugActor::StaticClass(), Settings);

		const bool bRemoved = Subsystem->RemoveDefaultSettings(ANDebugActor::StaticClass());
		CHECK_MESSAGE(TEXT("RemoveDefaultSettings should return true when the entry exists."), bRemoved)
		CHECK_FALSE_MESSAGE(TEXT("HasDefaultSettings should be false after removal."),
			Subsystem->HasDefaultSettings(ANDebugActor::StaticClass()))
	});
}

N_TEST_HIGH(UNActorPoolSubsystemTests_DefaultSettings_Update,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::DefaultSettings::Update",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that UpdateDefaultSettings modifies the stored settings and returns true.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		FNActorPoolSettings OriginalSettings;
		OriginalSettings.MinimumActorCount = 2;
		Subsystem->AddDefaultSettings(ANDebugActor::StaticClass(), OriginalSettings);

		FNActorPoolSettings UpdatedSettings;
		UpdatedSettings.MinimumActorCount = 7;
		const bool bUpdated = Subsystem->UpdateDefaultSettings(ANDebugActor::StaticClass(), UpdatedSettings);
		CHECK_MESSAGE(TEXT("UpdateDefaultSettings should return true when the entry exists."), bUpdated)
		CHECK_EQUALS("GetDefaultSettings should reflect the updated MinimumActorCount.",
			Subsystem->GetDefaultSettings(ANDebugActor::StaticClass()).MinimumActorCount, 7)
	});
}

N_TEST_HIGH(UNActorPoolSubsystemTests_DefaultSettings_GetFallsBackToGlobal,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::DefaultSettings::GetFallsBackToGlobal",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that GetDefaultSettings returns the global UNActorPoolsSettings defaults when no
	// class-specific default has been registered.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		const FNActorPoolSettings GlobalDefaults = UNActorPoolsSettings::Get()->DefaultSettings;
		const FNActorPoolSettings Retrieved = Subsystem->GetDefaultSettings(ANDebugActor::StaticClass());
		CHECK_EQUALS("GetDefaultSettings for an unregistered class should match the global default minimum count.",
			Retrieved.MinimumActorCount, GlobalDefaults.MinimumActorCount)
		CHECK_EQUALS("GetDefaultSettings for an unregistered class should match the global default maximum count.",
			Retrieved.MaximumActorCount, GlobalDefaults.MaximumActorCount)
	});
}

N_TEST_HIGH(UNActorPoolSubsystemTests_OnActorPoolAdded_FiredOnCreate,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::OnActorPoolAdded::FiredOnCreate",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that OnActorPoolAdded fires exactly once per new pool and does not fire for duplicates.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		int32 FireCount = 0;
		Subsystem->OnActorPoolAdded.AddLambda([&FireCount](FNActorPool*)
		{
			FireCount++;
		});

		FNActorPoolSettings Settings;
		Subsystem->CreateActorPool(AActor::StaticClass(), Settings);
		CHECK_EQUALS("OnActorPoolAdded should fire exactly once after one CreateActorPool call.", FireCount, 1)

		// A second call for the same class must not fire the delegate again.
		Subsystem->CreateActorPool(AActor::StaticClass(), Settings);
		CHECK_EQUALS("OnActorPoolAdded must not fire again when the pool already exists.", FireCount, 1)
	});
}

N_TEST_HIGH(UNActorPoolSubsystemTests_OnActorPoolAdded_FiredOnLazyGetActor,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::OnActorPoolAdded::FiredOnLazyGetActor",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that OnActorPoolAdded fires when GetActor lazily creates a pool, and not on subsequent calls.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		int32 FireCount = 0;
		Subsystem->OnActorPoolAdded.AddLambda([&FireCount](FNActorPool*)
		{
			FireCount++;
		});

		Subsystem->GetActor<AActor>(ANDebugActor::StaticClass());
		CHECK_EQUALS("OnActorPoolAdded should fire once when GetActor lazily creates a pool.", FireCount, 1)

		Subsystem->GetActor<AActor>(ANDebugActor::StaticClass());
		CHECK_EQUALS("OnActorPoolAdded must not fire again when the pool already exists.", FireCount, 1)
	});
}

#endif //WITH_TESTS

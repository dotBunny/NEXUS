// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NActorPool.h"
#include "NActorPoolSubsystem.h"
#include "Developer/NDebugActor.h"
#include "Developer/NObjectSnapshotUtils.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_CRITICAL(UNActorPoolSubsystemTests_WorldTeardown_NoAssertsOnEndPlay,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::WorldTeardown::NoAssertsOnEndPlay",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that a populated UNActorPoolSubsystem (filled pools + checked-out actors across
	// multiple classes) tears down through OnWorldEndPlay without firing asserts and without
	// leaking UObjects.
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
		CHECK_MESSAGE(TEXT("CreateActorPool should succeed during setup."), bCreated)

		AActor* Spawned = Subsystem->SpawnActor<AActor>(ANDebugActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Spawned == nullptr)
		{
			ADD_ERROR("SpawnActor returned nullptr during setup.");
			return;
		}

		AActor* Fetched = Subsystem->GetActor<AActor>(ANDebugActor::StaticClass());
		if (Fetched == nullptr)
		{
			ADD_ERROR("GetActor returned nullptr during setup.");
			return;
		}

		CHECK_MESSAGE(TEXT("HasActorPool should be true for the AActor pool before teardown."),
			Subsystem->HasActorPool(AActor::StaticClass()))
		CHECK_MESSAGE(TEXT("HasActorPool should be true for the ANDebugActor pool before teardown."),
			Subsystem->HasActorPool(ANDebugActor::StaticClass()))
	});
}

N_TEST_CRITICAL(UNActorPoolSubsystemTests_WorldTeardown_LevelTransitionSequence,
	"NEXUS::UnitTests::NActorPools::UNActorPoolSubsystem::WorldTeardown::LevelTransitionSequence",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that tearing down a populated world and immediately bringing up a second one - the
	// shape of a level transition from the subsystem's point of view - does not assert and does
	// not leak UObjects. The second world's subsystem must also start with no pools, proving
	// state did not survive the destroyed world.
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	const FNObjectSnapshot PreSnapshot = FNObjectSnapshotUtils::Snapshot();

	// First world: populate pools, check out actors, then let WorldTest drive teardown.
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from the first PIE world.");
			return;
		}

		FNActorPoolSettings Settings;
		Settings.MinimumActorCount = 2;
		Settings.MaximumActorCount = 5;
		Subsystem->CreateActorPool(AActor::StaticClass(), Settings);

		Subsystem->SpawnActor<AActor>(ANDebugActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		Subsystem->GetActor<AActor>(ANDebugActor::StaticClass());
	});

	// Second world: must come up clean and survive its own teardown.
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from the second PIE world.");
			return;
		}

		CHECK_FALSE_MESSAGE(TEXT("Second world's subsystem must not carry pools from the destroyed world."),
			Subsystem->HasActorPool(AActor::StaticClass()))
		CHECK_FALSE_MESSAGE(TEXT("Second world's subsystem must not carry pools from the destroyed world."),
			Subsystem->HasActorPool(ANDebugActor::StaticClass()))

		FNActorPoolSettings Settings;
		Subsystem->CreateActorPool(AActor::StaticClass(), Settings);
		AActor* Spawned = Subsystem->SpawnActor<AActor>(ANDebugActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Spawned == nullptr)
		{
			ADD_ERROR("SpawnActor returned nullptr in the second world.");
			return;
		}
		Subsystem->ReturnActor(Spawned);
	});

	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	const FNObjectSnapshot PostSnapshot = FNObjectSnapshotUtils::Snapshot();
	if (FNObjectSnapshotDiff Diff = FNObjectSnapshotUtils::Diff(PreSnapshot, PostSnapshot, true);
		Diff.AddedCount > 0)
	{
		for (int32 i = 0; i < Diff.AddedCount; i++)
		{
			ADD_ERROR(FString::Printf(TEXT("Leaked %s across level transition."), *Diff.Added[i].ToString()));
		}
	}
}

#endif //WITH_TESTS

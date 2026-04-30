// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NActorPool.h"
#include "NActorPoolSubsystem.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NActorPools::UNActorPoolSubsystemHarness
{
	constexpr int32 ObjectCount = 1000;
	constexpr int32 QueryCount  = 10000;

	constexpr float SpawnActorMaxDuration  = 3.0f;
	constexpr float ReturnActorMaxDuration = 3.0f;
	constexpr float GetActorPoolMaxDuration = 0.5f;
}

N_TEST_PERF(UNActorPoolSubsystemPerfTests_SpawnActor,
	"NEXUS::PerfTests::NActorPools::UNActorPoolSubsystem::SpawnActor",
	N_TEST_CONTEXT_EDITOR)
{
	// Measures the cost of routing ObjectCount Spawn calls through the subsystem.
	// The pool is pre-warmed outside the timed region to isolate routing overhead from actor creation.
	FNTestUtils::PrePerformanceTest();
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem) return;

		FNActorPoolSettings Settings;
		Settings.MinimumActorCount = 0;
		Settings.MaximumActorCount = NEXUS::PerfTests::NActorPools::UNActorPoolSubsystemHarness::ObjectCount;
		Settings.Strategy = ENActorPoolStrategy::Fixed;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);
		Subsystem->CreateActorPool(AActor::StaticClass(), Settings);

		// Pre-warm outside the timed region.
		Subsystem->GetActorPool(AActor::StaticClass())->Prewarm(NEXUS::PerfTests::NActorPools::UNActorPoolSubsystemHarness::ObjectCount);

		// TEST
		{
			N_TEST_TIMER_SCOPE(UNActorPoolSubsystemPerfTests_SpawnActor,
				NEXUS::PerfTests::NActorPools::UNActorPoolSubsystemHarness::SpawnActorMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NActorPools::UNActorPoolSubsystemHarness::ObjectCount; ++i)
			{
				Subsystem->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
			}
			NTestTimer.ManualStop();
		}
	}, true);
	FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(UNActorPoolSubsystemPerfTests_ReturnActor,
	"NEXUS::PerfTests::NActorPools::UNActorPoolSubsystem::ReturnActor",
	N_TEST_CONTEXT_EDITOR)
{
	// Measures the cost of routing ObjectCount Return calls through the subsystem.
	FNTestUtils::PrePerformanceTest();
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem) return;

		FNActorPoolSettings Settings;
		Settings.MinimumActorCount = 0;
		Settings.MaximumActorCount = NEXUS::PerfTests::NActorPools::UNActorPoolSubsystemHarness::ObjectCount;
		Settings.Strategy = ENActorPoolStrategy::Fixed;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);
		Subsystem->CreateActorPool(AActor::StaticClass(), Settings);

		Subsystem->GetActorPool(AActor::StaticClass())->Prewarm(NEXUS::PerfTests::NActorPools::UNActorPoolSubsystemHarness::ObjectCount);

		TArray<AActor*> Actors;
		Actors.Reserve(NEXUS::PerfTests::NActorPools::UNActorPoolSubsystemHarness::ObjectCount);
		for (int32 i = 0; i < NEXUS::PerfTests::NActorPools::UNActorPoolSubsystemHarness::ObjectCount; ++i)
		{
			Actors.Add(Subsystem->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator));
		}
		
		// TEST
		{
			N_TEST_TIMER_SCOPE(UNActorPoolSubsystemPerfTests_ReturnActor,
				NEXUS::PerfTests::NActorPools::UNActorPoolSubsystemHarness::ReturnActorMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NActorPools::UNActorPoolSubsystemHarness::ObjectCount; ++i)
			{
				Subsystem->ReturnActor(Actors[i]);
			}
			NTestTimer.ManualStop();
		}
		CHECK_MESSAGE(TEXT("Only 1 pool was made"), Subsystem->GetAllPools().Num() == 1)
	}, true);
	FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(UNActorPoolSubsystemPerfTests_GetActorPool,
	"NEXUS::PerfTests::NActorPools::UNActorPoolSubsystem::GetActorPool",
	N_TEST_CONTEXT_EDITOR)
{
	// Measures the cost of QueryCount TMap lookups via GetActorPool on an existing pool.
	// The result is consumed inside the loop and verified after to keep the calls observable.
	FNTestUtils::PrePerformanceTest();
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem) return;

		FNActorPoolSettings Settings;
		Subsystem->CreateActorPool(AActor::StaticClass(), Settings);

		FNActorPool* LastPool = nullptr;
		// TEST
		{
			N_TEST_TIMER_SCOPE(UNActorPoolSubsystemPerfTests_GetActorPool,
				NEXUS::PerfTests::NActorPools::UNActorPoolSubsystemHarness::GetActorPoolMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NActorPools::UNActorPoolSubsystemHarness::QueryCount; ++i)
			{
				LastPool = Subsystem->GetActorPool(AActor::StaticClass());
			}
			NTestTimer.ManualStop();
		}

		if (LastPool == nullptr)
		{
			ADD_ERROR("GetActorPool returned nullptr for a pool that was just created.");
			return;
		}
		CHECK_MESSAGE(TEXT("GetActorPool should return the AActor-templated pool."),
			LastPool->GetTemplate() == AActor::StaticClass())
	}, true);
	FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS

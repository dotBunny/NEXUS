// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NDynamicRef.h"
#include "NDynamicRefSubsystem.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::DynamicRefsTests::Perf
{
    constexpr int32 ObjectCount = 1000;
	constexpr int32 QueryCount = 100;
	
    constexpr float AddObjectMaxDuration = 0.65f;
    constexpr float RemoveObjectMaxDuration = 0.65f;
	
    constexpr float GetObjectsMaxDuration = 0.1f;
    constexpr float GetByNameMaxDuration = 0.1f;
}


N_TEST_PERF(FNDynamicRefsPerfTests_Subsystem_AddObject,
    "NEXUS::PerfTests::NDynamicRefs::Subsystem::AddObject",
    N_TEST_CONTEXT_EDITOR)
{
    // Measures the cost of adding ObjectCount unique actors to a single ENDynamicRef bucket.
    FNTestUtils::PrePerformanceTest();
    FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem) return;

        // Pre-spawn actors outside the timed region.
        TArray<AActor*> Actors;
        Actors.Reserve(NEXUS::DynamicRefsTests::Perf::ObjectCount);
        for (int32 i = 0; i < NEXUS::DynamicRefsTests::Perf::ObjectCount; ++i)
        {
            Actors.Add(World->SpawnActor<AActor>());
        }

        // TEST
        {
            N_TEST_TIMER_SCOPE(FNDynamicRefsPerfTests_Subsystem_AddObject,
                NEXUS::DynamicRefsTests::Perf::AddObjectMaxDuration)
            for (int32 i = 0; i < NEXUS::DynamicRefsTests::Perf::ObjectCount; ++i)
            {
                Subsystem->AddObject(NDR_Enemy_A, Actors[i]);
            }
            NTestTimer.ManualStop();
        }
    }, true);
    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNDynamicRefsPerfTests_Subsystem_RemoveObject,
    "NEXUS::PerfTests::NDynamicRefs::Subsystem::RemoveObject",
    N_TEST_CONTEXT_EDITOR)
{
    // Measures the cost of removing ObjectCount actors from a single ENDynamicRef bucket.
    FNTestUtils::PrePerformanceTest();
    FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem) return;

        TArray<AActor*> Actors;
        Actors.Reserve(NEXUS::DynamicRefsTests::Perf::ObjectCount);
        for (int32 i = 0; i < NEXUS::DynamicRefsTests::Perf::ObjectCount; ++i)
        {
            AActor* A = World->SpawnActor<AActor>();
            Subsystem->AddObject(NDR_Enemy_B, A);
            Actors.Add(A);
        }

        // TEST
        {
            N_TEST_TIMER_SCOPE(FNDynamicRefsPerfTests_Subsystem_RemoveObject,
                NEXUS::DynamicRefsTests::Perf::RemoveObjectMaxDuration)
            for (int32 i = 0; i < NEXUS::DynamicRefsTests::Perf::ObjectCount; ++i)
            {
                Subsystem->RemoveObject(NDR_Enemy_B, Actors[i]);
            }
            NTestTimer.ManualStop();
        }
    }, true);
    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNDynamicRefsPerfTests_Subsystem_GetObjects,
    "NEXUS::PerfTests::NDynamicRefs::Subsystem::GetObjects",
    N_TEST_CONTEXT_EDITOR)
{
    // Measures the cost of calling GetObjects ObjectCount times on a populated bucket.
    FNTestUtils::PrePerformanceTest();
    FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem) return;

        TArray<AActor*> Actors;
        Actors.Reserve(NEXUS::DynamicRefsTests::Perf::ObjectCount);
        for (int32 i = 0; i < NEXUS::DynamicRefsTests::Perf::ObjectCount; ++i)
        {
            AActor* A = World->SpawnActor<AActor>();
            Subsystem->AddObject(NDR_Enemy_C, A);
            Actors.Add(A);
        }

        // TEST
        {
            N_TEST_TIMER_SCOPE(FNDynamicRefsPerfTests_Subsystem_GetObjects,
                NEXUS::DynamicRefsTests::Perf::GetObjectsMaxDuration)
            for (int32 i = 0; i < NEXUS::DynamicRefsTests::Perf::QueryCount; ++i)
            {
                Subsystem->GetObjects(NDR_Enemy_C);
            }
            NTestTimer.ManualStop();
        }
    }, true);
    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNDynamicRefsPerfTests_Subsystem_GetFirstObject,
    "NEXUS::PerfTests::NDynamicRefs::Subsystem::GetFirstObject",
    N_TEST_CONTEXT_EDITOR)
{
    // Measures the cost of calling GetFirstObject ObjectCount times on a populated bucket.
    FNTestUtils::PrePerformanceTest();
    FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem) return;

        TArray<AActor*> Actors;
        Actors.Reserve(NEXUS::DynamicRefsTests::Perf::ObjectCount);
        for (int32 i = 0; i < NEXUS::DynamicRefsTests::Perf::ObjectCount; ++i)
        {
            AActor* A = World->SpawnActor<AActor>();
            Subsystem->AddObject(NDR_Enemy_D, A);
            Actors.Add(A);
        }

        // TEST
        {
            N_TEST_TIMER_SCOPE(FNDynamicRefsPerfTests_Subsystem_GetFirstObject,
                NEXUS::DynamicRefsTests::Perf::GetObjectsMaxDuration)
            for (int32 i = 0; i < NEXUS::DynamicRefsTests::Perf::QueryCount; ++i)
            {
                Subsystem->GetFirstObject(NDR_Enemy_D);
            }
            NTestTimer.ManualStop();
        }
    }, true);
    FNTestUtils::PostPerformanceTest();
}

// ─── NamedCollection (FName) ──────────────────────────────────────────────────

N_TEST_PERF(FNDynamicRefsPerfTests_Subsystem_AddObjectByName,
    "NEXUS::PerfTests::NDynamicRefs::Subsystem::AddObjectByName",
    N_TEST_CONTEXT_EDITOR)
{
    // Measures the cost of adding ObjectCount unique actors to a single named bucket.
    FNTestUtils::PrePerformanceTest();
    FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem) return;

        const FName TestName = TEXT("PerfTestAddByName");
        TArray<AActor*> Actors;
        Actors.Reserve(NEXUS::DynamicRefsTests::Perf::ObjectCount);
        for (int32 i = 0; i < NEXUS::DynamicRefsTests::Perf::ObjectCount; ++i)
        {
            Actors.Add(World->SpawnActor<AActor>());
        }

        // TEST
        {
            N_TEST_TIMER_SCOPE(FNDynamicRefsPerfTests_Subsystem_AddObjectByName,
                NEXUS::DynamicRefsTests::Perf::AddObjectMaxDuration)
            for (int32 i = 0; i < NEXUS::DynamicRefsTests::Perf::ObjectCount; ++i)
            {
                Subsystem->AddObjectByName(TestName, Actors[i]);
            }
            NTestTimer.ManualStop();
        }
    }, true);
    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNDynamicRefsPerfTests_Subsystem_RemoveObjectByName,
    "NEXUS::PerfTests::NDynamicRefs::Subsystem::RemoveObjectByName",
    N_TEST_CONTEXT_EDITOR)
{
    // Measures the cost of removing ObjectCount actors from a single named bucket.
    FNTestUtils::PrePerformanceTest();
    FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem) return;

        const FName TestName = TEXT("PerfTestRemoveByName");
        TArray<AActor*> Actors;
        Actors.Reserve(NEXUS::DynamicRefsTests::Perf::ObjectCount);
        for (int32 i = 0; i < NEXUS::DynamicRefsTests::Perf::ObjectCount; ++i)
        {
            AActor* A = World->SpawnActor<AActor>();
            Subsystem->AddObjectByName(TestName, A);
            Actors.Add(A);
        }

        // TEST
        {
            N_TEST_TIMER_SCOPE(FNDynamicRefsPerfTests_Subsystem_RemoveObjectByName,
                NEXUS::DynamicRefsTests::Perf::RemoveObjectMaxDuration)
            for (int32 i = 0; i < NEXUS::DynamicRefsTests::Perf::ObjectCount; ++i)
            {
                Subsystem->RemoveObjectByName(TestName, Actors[i]);
            }
            NTestTimer.ManualStop();
        }
    }, true);
    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNDynamicRefsPerfTests_Subsystem_GetObjectsByName,
    "NEXUS::PerfTests::NDynamicRefs::Subsystem::GetObjectsByName",
    N_TEST_CONTEXT_EDITOR)
{
    // Measures the cost of calling GetObjectsByName ObjectCount times on a populated named bucket.
    FNTestUtils::PrePerformanceTest();
    FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem) return;

        const FName TestName = TEXT("PerfTestGetByName");
        TArray<AActor*> Actors;
        Actors.Reserve(NEXUS::DynamicRefsTests::Perf::ObjectCount);
        for (int32 i = 0; i < NEXUS::DynamicRefsTests::Perf::ObjectCount; ++i)
        {
            AActor* A = World->SpawnActor<AActor>();
            Subsystem->AddObjectByName(TestName, A);
            Actors.Add(A);
        }

        // TEST
        {
            N_TEST_TIMER_SCOPE(FNDynamicRefsPerfTests_Subsystem_GetObjectsByName,
                NEXUS::DynamicRefsTests::Perf::GetByNameMaxDuration)
            for (int32 i = 0; i < NEXUS::DynamicRefsTests::Perf::QueryCount; ++i)
            {
                Subsystem->GetObjectsByName(TestName);
            }
            NTestTimer.ManualStop();
        }
    }, true);
    FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS

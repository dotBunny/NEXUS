// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NWidgetState.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NUI::FNWidgetStateHarness
{
    constexpr int32 EntryCount = 250;
    constexpr int32 QueryCount = 50;
    constexpr float AddMaxDuration = 0.1f;
    constexpr float GetMaxDuration = 0.05f;
    constexpr float OverlayMaxDuration = 0.5f;
}

N_TEST_PERF(FNWidgetStatePerfTests_AddString,
    "NEXUS::PerfTests::NUI::FNWidgetState::AddString",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNTestUtils::PrePerformanceTest();

    // Build keys outside the timed region.
    TArray<FString> Keys;
    Keys.Reserve(NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount);
    for (int32 i = 0; i < NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount; ++i)
    {
        Keys.Add(FString::Printf(TEXT("Key_%d"), i));
    }

    // ReSharper disable once CppTooWideScope
    FNWidgetState State;

    // TEST
    {
        N_TEST_TIMER_SCOPE(FNWidgetStatePerfTests_AddString, NEXUS::PerfTests::NUI::FNWidgetStateHarness::AddMaxDuration)
        for (int32 i = 0; i < NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount; ++i)
        {
            State.AddString(Keys[i], TEXT("Value"));
        }
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNWidgetStatePerfTests_AddBoolean,
    "NEXUS::PerfTests::NUI::FNWidgetState::AddBoolean",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNTestUtils::PrePerformanceTest();

    TArray<FString> Keys;
    Keys.Reserve(NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount);
    for (int32 i = 0; i < NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount; ++i)
    {
        Keys.Add(FString::Printf(TEXT("bKey_%d"), i));
    }

    // ReSharper disable once CppTooWideScope
    FNWidgetState State;

    // TEST
    {
        N_TEST_TIMER_SCOPE(FNWidgetStatePerfTests_AddBoolean, NEXUS::PerfTests::NUI::FNWidgetStateHarness::AddMaxDuration)
        for (int32 i = 0; i < NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount; ++i)
        {
            State.AddBoolean(Keys[i], (i % 2) == 0);
        }
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNWidgetStatePerfTests_AddFloat,
    "NEXUS::PerfTests::NUI::FNWidgetState::AddFloat",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNTestUtils::PrePerformanceTest();

    TArray<FString> Keys;
    Keys.Reserve(NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount);
    for (int32 i = 0; i < NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount; ++i)
    {
        Keys.Add(FString::Printf(TEXT("fKey_%d"), i));
    }

    // ReSharper disable once CppTooWideScope
    FNWidgetState State;

    // TEST
    {
        N_TEST_TIMER_SCOPE(FNWidgetStatePerfTests_AddFloat, NEXUS::PerfTests::NUI::FNWidgetStateHarness::AddMaxDuration)
        for (int32 i = 0; i < NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount; ++i)
        {
            State.AddFloat(Keys[i], static_cast<float>(i));
        }
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNWidgetStatePerfTests_GetString,
    "NEXUS::PerfTests::NUI::FNWidgetState::GetString",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Measures lookup cost on a fully populated state (worst-case linear scan).
    FNTestUtils::PrePerformanceTest();

    TArray<FString> Keys;
    Keys.Reserve(NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount);
    FNWidgetState State;
    for (int32 i = 0; i < NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount; ++i)
    {
        const FString Key = FString::Printf(TEXT("Key_%d"), i);
        Keys.Add(Key);
        State.AddString(Key, TEXT("Value"));
    }

    // TEST — always look up the last key (worst-case scan position).
    const FString& LastKey = Keys.Last();
    {
        N_TEST_TIMER_SCOPE(FNWidgetStatePerfTests_GetString, NEXUS::PerfTests::NUI::FNWidgetStateHarness::GetMaxDuration)
        for (int32 i = 0; i < NEXUS::PerfTests::NUI::FNWidgetStateHarness::QueryCount; ++i)
        {
            // ReSharper disable once CppExpressionWithoutSideEffects
            State.GetString(LastKey);
        }
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNWidgetStatePerfTests_GetBoolean,
    "NEXUS::PerfTests::NUI::FNWidgetState::GetBoolean",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNTestUtils::PrePerformanceTest();

    TArray<FString> Keys;
    Keys.Reserve(NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount);
    FNWidgetState State;
    for (int32 i = 0; i < NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount; ++i)
    {
        const FString Key = FString::Printf(TEXT("bKey_%d"), i);
        Keys.Add(Key);
        State.AddBoolean(Key, true);
    }

    const FString& LastKey = Keys.Last();
    {
        N_TEST_TIMER_SCOPE(FNWidgetStatePerfTests_GetBoolean, NEXUS::PerfTests::NUI::FNWidgetStateHarness::GetMaxDuration)
        for (int32 i = 0; i < NEXUS::PerfTests::NUI::FNWidgetStateHarness::QueryCount; ++i)
        {
            // ReSharper disable once CppExpressionWithoutSideEffects
            State.GetBoolean(LastKey);
        }
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNWidgetStatePerfTests_GetFloat,
    "NEXUS::PerfTests::NUI::FNWidgetState::GetFloat",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNTestUtils::PrePerformanceTest();

    TArray<FString> Keys;
    Keys.Reserve(NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount);
    FNWidgetState State;
    for (int32 i = 0; i < NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount; ++i)
    {
        const FString Key = FString::Printf(TEXT("fKey_%d"), i);
        Keys.Add(Key);
        State.AddFloat(Key, static_cast<float>(i));
    }

    const FString& LastKey = Keys.Last();
    {
        N_TEST_TIMER_SCOPE(FNWidgetStatePerfTests_GetFloat, NEXUS::PerfTests::NUI::FNWidgetStateHarness::GetMaxDuration)
        for (int32 i = 0; i < NEXUS::PerfTests::NUI::FNWidgetStateHarness::QueryCount; ++i)
        {
            // ReSharper disable once CppExpressionWithoutSideEffects
            State.GetFloat(LastKey);
        }
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNWidgetStatePerfTests_OverlayState_Merge,
    "NEXUS::PerfTests::NUI::FNWidgetState::OverlayState::Merge",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Measures the cost of merging two states each with EntryCount entries (no replace).
    FNTestUtils::PrePerformanceTest();

    FNWidgetState Base;
    FNWidgetState Overlay;
    for (int32 i = 0; i < NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount; ++i)
    {
        Base.AddString(FString::Printf(TEXT("Base_%d"), i), TEXT("B"));
        Overlay.AddString(FString::Printf(TEXT("Overlay_%d"), i), TEXT("O"));
    }

    // TEST
    {
        N_TEST_TIMER_SCOPE(FNWidgetStatePerfTests_OverlayState_Merge, NEXUS::PerfTests::NUI::FNWidgetStateHarness::OverlayMaxDuration)
        Base.OverlayState(Overlay, false);
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNWidgetStatePerfTests_OverlayState_Replace,
    "NEXUS::PerfTests::NUI::FNWidgetState::OverlayState::Replace",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Measures the cost of merging two states with full key overlap (replace mode).
    FNTestUtils::PrePerformanceTest();

    FNWidgetState Base;
    FNWidgetState Overlay;
    for (int32 i = 0; i < NEXUS::PerfTests::NUI::FNWidgetStateHarness::EntryCount; ++i)
    {
        const FString Key = FString::Printf(TEXT("Key_%d"), i);
        Base.AddString(Key, TEXT("Original"));
        Overlay.AddString(Key, TEXT("Replacement"));
    }

    // TEST
    {
        N_TEST_TIMER_SCOPE(FNWidgetStatePerfTests_OverlayState_Replace, NEXUS::PerfTests::NUI::FNWidgetStateHarness::OverlayMaxDuration)
        Base.OverlayState(Overlay, true);
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS

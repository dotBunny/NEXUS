// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NWidgetState.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::UITests::Perf
{
    constexpr int32 EntryCount = 250;
	constexpr int32 QueryCount = 50;
    constexpr float AddMaxDuration = 0.1f;
    constexpr float GetMaxDuration = 0.05f;
    constexpr float OverlayMaxDuration = 0.5f;
}

N_TEST_PERF(FNUIPerfTests_WidgetState_AddString,
    "NEXUS::PerfTests::NUI::WidgetState::AddString",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNTestUtils::PrePerformanceTest();

    // Build keys outside the timed region.
    TArray<FString> Keys;
    Keys.Reserve(NEXUS::UITests::Perf::EntryCount);
    for (int32 i = 0; i < NEXUS::UITests::Perf::EntryCount; ++i)
    {
        Keys.Add(FString::Printf(TEXT("Key_%d"), i));
    }

	// ReSharper disable once CppTooWideScope
    FNWidgetState State;

    // TEST
    {
        N_TEST_TIMER_SCOPE(FNUIPerfTests_WidgetState_AddString, NEXUS::UITests::Perf::AddMaxDuration)
        for (int32 i = 0; i < NEXUS::UITests::Perf::EntryCount; ++i)
        {
            State.AddString(Keys[i], TEXT("Value"));
        }
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNUIPerfTests_WidgetState_AddBoolean,
    "NEXUS::PerfTests::NUI::WidgetState::AddBoolean",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNTestUtils::PrePerformanceTest();

    TArray<FString> Keys;
    Keys.Reserve(NEXUS::UITests::Perf::EntryCount);
    for (int32 i = 0; i < NEXUS::UITests::Perf::EntryCount; ++i)
    {
        Keys.Add(FString::Printf(TEXT("bKey_%d"), i));
    }

    // ReSharper disable once CppTooWideScope
    FNWidgetState State;

    // TEST
    {
        N_TEST_TIMER_SCOPE(FNUIPerfTests_WidgetState_AddBoolean, NEXUS::UITests::Perf::AddMaxDuration)
        for (int32 i = 0; i < NEXUS::UITests::Perf::EntryCount; ++i)
        {
            State.AddBoolean(Keys[i], (i % 2) == 0);
        }
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNUIPerfTests_WidgetState_AddFloat,
    "NEXUS::PerfTests::NUI::WidgetState::AddFloat",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNTestUtils::PrePerformanceTest();

    TArray<FString> Keys;
    Keys.Reserve(NEXUS::UITests::Perf::EntryCount);
    for (int32 i = 0; i < NEXUS::UITests::Perf::EntryCount; ++i)
    {
        Keys.Add(FString::Printf(TEXT("fKey_%d"), i));
    }

	// ReSharper disable once CppTooWideScope
    FNWidgetState State;

    // TEST
    {
        N_TEST_TIMER_SCOPE(FNUIPerfTests_WidgetState_AddFloat, NEXUS::UITests::Perf::AddMaxDuration)
        for (int32 i = 0; i < NEXUS::UITests::Perf::EntryCount; ++i)
        {
            State.AddFloat(Keys[i], static_cast<float>(i));
        }
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNUIPerfTests_WidgetState_GetString,
    "NEXUS::PerfTests::NUI::WidgetState::GetString",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Measures lookup cost on a fully populated state (worst-case linear scan).
    FNTestUtils::PrePerformanceTest();

    TArray<FString> Keys;
    Keys.Reserve(NEXUS::UITests::Perf::EntryCount);
    FNWidgetState State;
    for (int32 i = 0; i < NEXUS::UITests::Perf::EntryCount; ++i)
    {
        const FString Key = FString::Printf(TEXT("Key_%d"), i);
        Keys.Add(Key);
        State.AddString(Key, TEXT("Value"));
    }

    // TEST — always look up the last key (worst-case scan position).
    const FString& LastKey = Keys.Last();
    {
        N_TEST_TIMER_SCOPE(FNUIPerfTests_WidgetState_GetString, NEXUS::UITests::Perf::GetMaxDuration)
        for (int32 i = 0; i < NEXUS::UITests::Perf::QueryCount; ++i)
        {
	        // ReSharper disable once CppExpressionWithoutSideEffects
	        State.GetString(LastKey);
        }
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNUIPerfTests_WidgetState_GetBoolean,
    "NEXUS::PerfTests::NUI::WidgetState::GetBoolean",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNTestUtils::PrePerformanceTest();

    TArray<FString> Keys;
    Keys.Reserve(NEXUS::UITests::Perf::EntryCount);
    FNWidgetState State;
    for (int32 i = 0; i < NEXUS::UITests::Perf::EntryCount; ++i)
    {
        const FString Key = FString::Printf(TEXT("bKey_%d"), i);
        Keys.Add(Key);
        State.AddBoolean(Key, true);
    }

    const FString& LastKey = Keys.Last();
    {
        N_TEST_TIMER_SCOPE(FNUIPerfTests_WidgetState_GetBoolean, NEXUS::UITests::Perf::GetMaxDuration)
        for (int32 i = 0; i < NEXUS::UITests::Perf::QueryCount; ++i)
        {
	        // ReSharper disable once CppExpressionWithoutSideEffects
	        State.GetBoolean(LastKey);
        }
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNUIPerfTests_WidgetState_GetFloat,
    "NEXUS::PerfTests::NUI::WidgetState::GetFloat",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNTestUtils::PrePerformanceTest();

    TArray<FString> Keys;
    Keys.Reserve(NEXUS::UITests::Perf::EntryCount);
    FNWidgetState State;
    for (int32 i = 0; i < NEXUS::UITests::Perf::EntryCount; ++i)
    {
        const FString Key = FString::Printf(TEXT("fKey_%d"), i);
        Keys.Add(Key);
        State.AddFloat(Key, static_cast<float>(i));
    }

    const FString& LastKey = Keys.Last();
    {
        N_TEST_TIMER_SCOPE(FNUIPerfTests_WidgetState_GetFloat, NEXUS::UITests::Perf::GetMaxDuration)
        for (int32 i = 0; i < NEXUS::UITests::Perf::QueryCount; ++i)
        {
	        // ReSharper disable once CppExpressionWithoutSideEffects
	        State.GetFloat(LastKey);
        }
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNUIPerfTests_WidgetState_OverlayState_Merge,
    "NEXUS::PerfTests::NUI::WidgetState::OverlayState::Merge",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Measures the cost of merging two states each with EntryCount entries (no replace).
    FNTestUtils::PrePerformanceTest();

    FNWidgetState Base;
    FNWidgetState Overlay;
    for (int32 i = 0; i < NEXUS::UITests::Perf::EntryCount; ++i)
    {
        Base.AddString(FString::Printf(TEXT("Base_%d"), i), TEXT("B"));
        Overlay.AddString(FString::Printf(TEXT("Overlay_%d"), i), TEXT("O"));
    }

    // TEST
    {
        N_TEST_TIMER_SCOPE(FNUIPerfTests_WidgetState_OverlayState_Merge, NEXUS::UITests::Perf::OverlayMaxDuration)
        Base.OverlayState(Overlay, false);
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNUIPerfTests_WidgetState_OverlayState_Replace,
    "NEXUS::PerfTests::NUI::WidgetState::OverlayState::Replace",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Measures the cost of merging two states with full key overlap (replace mode).
    FNTestUtils::PrePerformanceTest();

    FNWidgetState Base;
    FNWidgetState Overlay;
    for (int32 i = 0; i < NEXUS::UITests::Perf::EntryCount; ++i)
    {
        const FString Key = FString::Printf(TEXT("Key_%d"), i);
        Base.AddString(Key, TEXT("Original"));
        Overlay.AddString(Key, TEXT("Replacement"));
    }

    // TEST
    {
        N_TEST_TIMER_SCOPE(FNUIPerfTests_WidgetState_OverlayState_Replace, NEXUS::UITests::Perf::OverlayMaxDuration)
        Base.OverlayState(Overlay, true);
        NTestTimer.ManualStop();
    }

    FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS

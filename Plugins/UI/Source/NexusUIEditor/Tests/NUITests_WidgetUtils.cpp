// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWidgetUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_CRITICAL(FNUITests_WidgetUtils_ToEVisibility_Visible,
    "NEXUS::UnitTests::NUI::WidgetUtils::ToEVisibility::Visible",
    N_TEST_CONTEXT_ANYWHERE)
{
    CHECK_EQUALS("ESlateVisibility::Visible should map to EVisibility::Visible.",
        FNWidgetUtils::ToEVisibility(ESlateVisibility::Visible),
        EVisibility::Visible)
}

N_TEST_CRITICAL(FNUITests_WidgetUtils_ToEVisibility_Collapsed,
    "NEXUS::UnitTests::NUI::WidgetUtils::ToEVisibility::Collapsed",
    N_TEST_CONTEXT_ANYWHERE)
{
    CHECK_EQUALS("ESlateVisibility::Collapsed should map to EVisibility::Collapsed.",
        FNWidgetUtils::ToEVisibility(ESlateVisibility::Collapsed),
        EVisibility::Collapsed)
}

N_TEST_CRITICAL(FNUITests_WidgetUtils_ToEVisibility_Hidden,
    "NEXUS::UnitTests::NUI::WidgetUtils::ToEVisibility::Hidden",
    N_TEST_CONTEXT_ANYWHERE)
{
    CHECK_EQUALS("ESlateVisibility::Hidden should map to EVisibility::Hidden.",
        FNWidgetUtils::ToEVisibility(ESlateVisibility::Hidden),
        EVisibility::Hidden)
}

N_TEST_CRITICAL(FNUITests_WidgetUtils_ToEVisibility_HitTestInvisible,
    "NEXUS::UnitTests::NUI::WidgetUtils::ToEVisibility::HitTestInvisible",
    N_TEST_CONTEXT_ANYWHERE)
{
    CHECK_EQUALS("ESlateVisibility::HitTestInvisible should map to EVisibility::HitTestInvisible.",
        FNWidgetUtils::ToEVisibility(ESlateVisibility::HitTestInvisible),
        EVisibility::HitTestInvisible)
}

N_TEST_CRITICAL(FNUITests_WidgetUtils_ToEVisibility_SelfHitTestInvisible,
    "NEXUS::UnitTests::NUI::WidgetUtils::ToEVisibility::SelfHitTestInvisible",
    N_TEST_CONTEXT_ANYWHERE)
{
    CHECK_EQUALS("ESlateVisibility::SelfHitTestInvisible should map to EVisibility::SelfHitTestInvisible.",
        FNWidgetUtils::ToEVisibility(ESlateVisibility::SelfHitTestInvisible),
        EVisibility::SelfHitTestInvisible)
}

#endif //WITH_TESTS

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_EDITORONLY_DATA

#include "NInterfaceMacrosTestTypes.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(NInterfaceMacrosTests_InvokeZeroArg_RoutesToImplementer,
	"NEXUS::UnitTests::NCore::NInterfaceMacros::InvokeZeroArg_RoutesToImplementer",
	N_TEST_CONTEXT_ANYWHERE)
{
	UNInterfaceMacrosTestImplementer* Subject = NewObject<UNInterfaceMacrosTestImplementer>();
	FNInterfaceMacrosTestDispatcher::InvokeOnZeroArg(Subject);
	CHECK_EQUALS("Zero-arg invoke should hit the implementer's override exactly once.", Subject->ZeroArgHits, 1)
}

N_TEST_HIGH(NInterfaceMacrosTests_InvokeOneParam_ForwardsValue,
	"NEXUS::UnitTests::NCore::NInterfaceMacros::InvokeOneParam_ForwardsValue",
	N_TEST_CONTEXT_ANYWHERE)
{
	UNInterfaceMacrosTestImplementer* Subject = NewObject<UNInterfaceMacrosTestImplementer>();
	FNInterfaceMacrosTestDispatcher::InvokeOnOneArg(Subject, 42);
	CHECK_EQUALS("One-param invoke should hit the implementer's override exactly once.", Subject->OneArgHits, 1)
	CHECK_EQUALS("One-param invoke should forward the int argument.", Subject->OneArgValue, 42)
}

N_TEST_HIGH(NInterfaceMacrosTests_InvokeTwoParams_ForwardsValues,
	"NEXUS::UnitTests::NCore::NInterfaceMacros::InvokeTwoParams_ForwardsValues",
	N_TEST_CONTEXT_ANYWHERE)
{
	UNInterfaceMacrosTestImplementer* Subject = NewObject<UNInterfaceMacrosTestImplementer>();
	FNInterfaceMacrosTestDispatcher::InvokeOnTwoArgs(Subject, 7, TEXT("nexus"));
	CHECK_EQUALS("Two-params invoke should hit the implementer's override exactly once.", Subject->TwoArgsHits, 1)
	CHECK_EQUALS("Two-params invoke should forward the int argument.", Subject->TwoArgsValue, 7)
	CHECK_MESSAGE(TEXT("Two-params invoke should forward the string argument."), Subject->TwoArgsText == TEXT("nexus"))
}

N_TEST_MEDIUM(NInterfaceMacrosTests_InvokeOnNonImplementer_IsNoOp,
	"NEXUS::UnitTests::NCore::NInterfaceMacros::InvokeOnNonImplementer_IsNoOp",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that the Implements<> guard short-circuits every variant when the target does not implement the interface.
	UNInterfaceMacrosTestNonImplementer* Subject = NewObject<UNInterfaceMacrosTestNonImplementer>();
	CHECK_FALSE_MESSAGE(TEXT("Test premise: subject must not implement the test interface."),
		Subject->Implements<UNInterfaceMacrosTestSubject>())

	FNInterfaceMacrosTestDispatcher::InvokeOnZeroArg(Subject);
	FNInterfaceMacrosTestDispatcher::InvokeOnOneArg(Subject, 99);
	FNInterfaceMacrosTestDispatcher::InvokeOnTwoArgs(Subject, 99, TEXT("ignored"));
}

N_TEST_MEDIUM(NInterfaceMacrosTests_InvokeAcrossArities_AccumulatesIndependently,
	"NEXUS::UnitTests::NCore::NInterfaceMacros::InvokeAcrossArities_AccumulatesIndependently",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that successive invokes against the same instance hit their respective overrides without cross-talk.
	UNInterfaceMacrosTestImplementer* Subject = NewObject<UNInterfaceMacrosTestImplementer>();

	FNInterfaceMacrosTestDispatcher::InvokeOnZeroArg(Subject);
	FNInterfaceMacrosTestDispatcher::InvokeOnZeroArg(Subject);
	FNInterfaceMacrosTestDispatcher::InvokeOnOneArg(Subject, -3);
	FNInterfaceMacrosTestDispatcher::InvokeOnTwoArgs(Subject, 11, TEXT("payload"));

	CHECK_EQUALS("Zero-arg counter should reflect both invocations.", Subject->ZeroArgHits, 2)
	CHECK_EQUALS("One-arg counter should reflect the single invocation.", Subject->OneArgHits, 1)
	CHECK_EQUALS("One-arg value should match the last forwarded int.", Subject->OneArgValue, -3)
	CHECK_EQUALS("Two-args counter should reflect the single invocation.", Subject->TwoArgsHits, 1)
	CHECK_EQUALS("Two-args int should match the last forwarded value.", Subject->TwoArgsValue, 11)
	CHECK_MESSAGE(TEXT("Two-args string should match the last forwarded text."), Subject->TwoArgsText == TEXT("payload"))
}

#endif // WITH_EDITORONLY_DATA

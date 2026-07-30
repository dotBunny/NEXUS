// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_TESTS

/**
 * A minimal, non-UObject carrier used by unit tests that need a simple, stack-allocatable payload.
 *
 * Intentionally has no constructors, virtuals or invariants — tests populate it directly.
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/developer/test-object/">FNTestObject</a>
 */
class FNTestObject
{
public:
	int32 Counter;
	FString Message;
	uint8 State;
};

#endif // WITH_TESTS
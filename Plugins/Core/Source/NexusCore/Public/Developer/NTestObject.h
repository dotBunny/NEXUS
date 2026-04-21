// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * A minimal, non-UObject carrier used by unit tests that need a simple, stack-allocatable payload.
 *
 * Intentionally has no constructors, virtuals or invariants — tests populate it directly.
 */
class FNTestObject
{
public:
	int Counter;
	FString Message;
	uint8 State;
};

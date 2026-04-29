// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NTestUObject.generated.h"

/**
 * A minimal, hidden UObject used by tests that need a real UObject to exercise GC or reflection paths.
 *
 * Kept deliberately tiny and unreferenced so it can be created, collected and inspected without
 * dragging in gameplay dependencies. Not intended for gameplay use.
 */
UCLASS(NotPlaceable, Hidden, HideDropdown, MinimalAPI)
class UNTestUObject : public UObject
{
	GENERATED_BODY()

public:
	int32 Counter;
	FString Message;
	uint8 State;
};

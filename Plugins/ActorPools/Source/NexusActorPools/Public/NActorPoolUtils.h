// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "INActorPoolItem.h"

/**
 * Static helper utilities for the Actor Pool system.
 */
class FNActorPoolUtils
{
public:
	/**
	 * Convert an ENActorOperationalState to a human-readable string.
	 * @param State The operational state to convert.
	 * @return The string representation of the state (e.g. "Enabled", "Disabled").
	 */
	static FString ToString(const ENActorOperationalState& State)
	{
		switch (State)
		{
			using enum ENActorOperationalState;
		case Undefined:
			return FString("Undefined");
		case Created:
			return FString("Created");
		case Enabled:
			return FString("Enabled");
		case Disabled:
			return FString("Disabled");
		case Released:
			return FString("Released");
		}
		return FString("Undefined");
	}
};
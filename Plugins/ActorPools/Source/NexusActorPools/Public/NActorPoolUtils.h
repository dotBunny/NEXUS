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
		case AOS_Undefined:
			return FString("Undefined");
		case AOS_Created:
			return FString("Created");
		case AOS_Enabled:
			return FString("Enabled");
		case AOS_Disabled:
			return FString("Disabled");
		case AOS_Destroyed:
			return FString("Destroyed");
		}
		return FString("Undefined");
	}
};

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "INActorPoolItem.h"

class FNActorPoolUtils
{
public:
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

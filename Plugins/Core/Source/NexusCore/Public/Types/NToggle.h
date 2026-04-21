// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

/**
 * An enumeration representing a boolean value, with a default option.
 */
UENUM(BlueprintType)
enum ENToggle : int8
{
	T_Default = -1		UMETA(DisplayName = "Default", Description = "Take no action"),
	T_False = 0			UMETA(DisplayName = "False"),
	T_True = 1			UMETA(DisplayName = "True")
};

/**
 * A utility class for operating on ENToggle.
 */
class NEXUSCORE_API FNToggle
{
public:
	/**
	 * Returns a human-readable name for InToggle.
	 * @param InToggle The enum value to stringify.
	 * @return "Default", "False", "True", or "Unknown" for unrecognized values.
	 */
	static FString ToString(const ENToggle& InToggle)
	{
		switch (InToggle)
		{
			case T_Default: return TEXT("Default");
			case T_False: return TEXT("False");
			case T_True: return TEXT("True");
			default: return TEXT("Unknown");
		}
	}
};
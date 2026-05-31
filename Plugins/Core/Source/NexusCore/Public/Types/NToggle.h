// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

/**
 * An enumeration representing a boolean value, with a default option.
 */
UENUM(BlueprintType)
enum class ENToggle : uint8
{
	Default = 0			UMETA(DisplayName = "Default", Description = "Take no action"),
	Disabled = 1		UMETA(DisplayName = "Disabled"),
	Enabled = 2			UMETA(DisplayName = "Enabled")
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
	 * @return "Default", "Disabled", "Enabled", or "Unknown" for unrecognized values.
	 */
	static FString ToString(const ENToggle& InToggle)
	{
		switch (InToggle)
		{
			using enum ENToggle;
			case Default: return TEXT("Default");
			case Disabled: return TEXT("Disabled");
			case Enabled: return TEXT("Enabled");
			default: return TEXT("Unknown");
		}
	}
};
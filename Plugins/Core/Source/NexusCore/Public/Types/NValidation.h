// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NValidation.generated.h"

UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENBakedValidation : uint8
{
	None = 0 UMETA(Hidden),
	Baked = 1,
	UpToDate = 2
};
ENUM_CLASS_FLAGS(ENBakedValidation)

#define D_BAKED_VALIDATION_ALL static_cast<uint8>(ENBakedValidation::Baked) | static_cast<uint8>(ENBakedValidation::UpToDate)

UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENGeneratedValidation : uint8
{
	None = 0 UMETA(Hidden),
	Generated = 1,
	UpToDate = 2
};
ENUM_CLASS_FLAGS(ENGeneratedValidation)

#define D_GENERATED_VALIDATION_ALL static_cast<uint8>(ENGeneratedValidation::Generated) | static_cast<uint8>(ENGeneratedValidation::UpToDate)
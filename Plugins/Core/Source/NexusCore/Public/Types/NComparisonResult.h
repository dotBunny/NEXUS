// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

UENUM(BlueprintType)
enum class ENComparisonResult : uint8
{
	Equal = 0,
	GreaterThan = 1,
	GreaterThanOrEqual = 2	UMETA(DisplayName = "Greater Than Or Equal"),
	LessThan= 3,
	LessThanOrEqual = 4		UMETA(DisplayName = "Less Than Or Equal"),
	NotEqual = 5
};
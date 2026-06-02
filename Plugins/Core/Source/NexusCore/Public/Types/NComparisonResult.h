// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

UENUM(BlueprintType)
enum class ENComparisonResult : uint8
{
	Equal = 0 ,
	GreaterThan = 1,
	LessThan= 2,
	NotEqual = 3
};
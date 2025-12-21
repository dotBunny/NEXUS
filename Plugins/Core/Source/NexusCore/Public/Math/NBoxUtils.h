// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

/**
 * A collection of utility methods for working with FBoxes.
 */
class NEXUSCORE_API FNBoxUtils
{
public:
	static TArray<FVector> GetVertices(const FBox& Box);
	
};
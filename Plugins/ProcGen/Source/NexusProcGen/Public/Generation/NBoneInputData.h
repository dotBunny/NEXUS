// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Types/NCardinalRotation.h"

struct NEXUSPROCGEN_API FNBoneInputData
{
	FVector WorldPosition;
	
	FVector MinimumPoint;
	FVector MaximumPoint;
	
	FIntVector2 SocketSize;
	
	FNCardinalRotation CardinalRotation;
};

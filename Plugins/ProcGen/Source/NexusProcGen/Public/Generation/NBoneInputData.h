// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

struct NEXUSPROCGEN_API FNBoneInputData
{
	FVector WorldPosition;
	FRotator WorldRotation;
	
	FVector MinimumPoint;
	FVector MaximumPoint;
	
	FIntVector2 SocketSize;
};

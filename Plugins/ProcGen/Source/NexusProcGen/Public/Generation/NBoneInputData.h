// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

struct NEXUSPROCGEN_API FNBoneInputData
{
	FVector WorldPosition;
	FRotator WorldRotation;
	
	TArray<FVector> CornerPoints;
	
	FIntVector2 SocketSize;
};

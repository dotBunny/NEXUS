// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Snapshot of a bone's spatial data, copied onto a task so it can be consumed off the game thread
 * without touching the live UNBoneComponent.
 */
struct NEXUSPROCGEN_API FNVirtualBoneData
{
	/** World-space location of the bone at the time of snapshot. */
	FVector WorldPosition;

	/** World-space rotation of the bone at the time of snapshot. */
	FRotator WorldRotation;

	/** Cached world-space corners of the bone's junction socket. */
	TArray<FVector> CornerPoints;

	/** Bone socket size in grid units (width, height). */
	FIntVector2 SocketSize;
};

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * A collection of utility methods for working with AVolumes.
 */
class NEXUSCORE_API FNBoundsUtils
{
public:
	static FORCEINLINE bool IsBoundsContainedInBounds(const FBoxSphereBounds& InnerBounds, const FBoxSphereBounds& OuterBounds)
	{
		// Check if the inner box is completely contained within the outer box
		const FVector InnerMin = InnerBounds.Origin - InnerBounds.BoxExtent;
		const FVector InnerMax = InnerBounds.Origin + InnerBounds.BoxExtent;
		const FVector OuterMin = OuterBounds.Origin - OuterBounds.BoxExtent;
		const FVector OuterMax = OuterBounds.Origin + OuterBounds.BoxExtent;
    
		return (InnerMin.X >= OuterMin.X && InnerMin.Y >= OuterMin.Y && InnerMin.Z >= OuterMin.Z &&
				InnerMax.X <= OuterMax.X && InnerMax.Y <= OuterMax.Y && InnerMax.Z <= OuterMax.Z);
	}
	static bool IsVolumeContainedInVolume(const AVolume* InnerVolume, const AVolume* OuterVolume);
	static bool IsVolumeContainedInVolumeFast(const AVolume* InnerVolume, const AVolume* OuterVolume);
};
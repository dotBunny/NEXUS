// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NBoundsUtils.h"

bool FNBoundsUtils::IsVolumeContainedInVolume(const AVolume* InnerVolume, const AVolume* OuterVolume)
{
	if (!InnerVolume || !OuterVolume)
	{
		return false;
	}

	// Get the bounds of the inner volume
	FBoxSphereBounds InnerBounds = InnerVolume->GetBounds();
    
	// Check if the outer volume encompasses the center and extends of the inner volume
	const FVector InnerCenter = InnerBounds.Origin;
	const FVector InnerExtent = InnerBounds.BoxExtent;
    
	// Check key points of the inner volume's bounding box
	TArray<FVector> TestPoints;
	TestPoints.Add(InnerCenter + FVector(InnerExtent.X, InnerExtent.Y, InnerExtent.Z));
	TestPoints.Add(InnerCenter + FVector(-InnerExtent.X, InnerExtent.Y, InnerExtent.Z));
	TestPoints.Add(InnerCenter + FVector(InnerExtent.X, -InnerExtent.Y, InnerExtent.Z));
	TestPoints.Add(InnerCenter + FVector(InnerExtent.X, InnerExtent.Y, -InnerExtent.Z));
	TestPoints.Add(InnerCenter + FVector(-InnerExtent.X, -InnerExtent.Y, InnerExtent.Z));
	TestPoints.Add(InnerCenter + FVector(-InnerExtent.X, InnerExtent.Y, -InnerExtent.Z));
	TestPoints.Add(InnerCenter + FVector(InnerExtent.X, -InnerExtent.Y, -InnerExtent.Z));
	TestPoints.Add(InnerCenter + FVector(-InnerExtent.X, -InnerExtent.Y, -InnerExtent.Z));
    
	// Check if all corner points are encompassed by the outer volume
	for (const FVector& Point : TestPoints)
	{
		if (!OuterVolume->EncompassesPoint(Point))
		{
			return false;
		}
	}
    
	return true;
}

bool FNBoundsUtils::IsVolumeContainedInVolumeFast(const AVolume* InnerVolume, const AVolume* OuterVolume)
{
	if (!InnerVolume || !OuterVolume)
	{
		return false;
	}

	FBoxSphereBounds InnerBounds = InnerVolume->GetBounds();
	FBoxSphereBounds OuterBounds = OuterVolume->GetBounds();
    
	// Check if the inner box is completely contained within the outer box
	const FVector InnerMin = InnerBounds.Origin - InnerBounds.BoxExtent;
	const FVector InnerMax = InnerBounds.Origin + InnerBounds.BoxExtent;
	const FVector OuterMin = OuterBounds.Origin - OuterBounds.BoxExtent;
	const FVector OuterMax = OuterBounds.Origin + OuterBounds.BoxExtent;
    
	return (InnerMin.X >= OuterMin.X && InnerMin.Y >= OuterMin.Y && InnerMin.Z >= OuterMin.Z &&
			InnerMax.X <= OuterMax.X && InnerMax.Y <= OuterMax.Y && InnerMax.Z <= OuterMax.Z);
}

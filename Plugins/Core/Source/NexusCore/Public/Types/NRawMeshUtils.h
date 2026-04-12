// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRawMesh.h"

class NEXUSCORE_API FNRawMeshUtils
{
public:
	static bool DoesIntersect(
		const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
		const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation);
	
	static bool IsRelativePointInside(const FNRawMesh& Mesh, const FVector& RelativePoint);
	
	static bool AnyRelativePointsInside(const FNRawMesh& Mesh, const TArray<FVector>& RelativePoints);
	
private:

	static bool DoesIntersectTriangles(
		const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
		const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation);
};
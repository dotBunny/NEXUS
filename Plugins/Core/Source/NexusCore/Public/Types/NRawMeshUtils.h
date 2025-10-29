// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRawMesh.h"

class FNRawMeshUtils
{
public:
	static bool DoesIntersect(
		FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
		FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation);
private:
	static bool DoesIntersectTriangles(
		FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
		FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation);
};
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNTriangleUtils
{
public:
	static bool IsPointInTriangle(const FVector& Point, const FVector& A, const FVector& B, const FVector& C)
	{
		const FVector v0 = C - A;
		const FVector v1 = B - A;
		const FVector v2 = Point - A;

		const float dot00 = FVector::DotProduct(v0, v0);
		const float dot01 = FVector::DotProduct(v0, v1);
		const float dot02 = FVector::DotProduct(v0, v2);
		const float dot11 = FVector::DotProduct(v1, v1);
		const float dot12 = FVector::DotProduct(v1, v2);

		const float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
		const float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
		const float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

		return (u >= 0.0f) && (v >= 0.0f) && (u + v <= 1.0f);
	}

	/**
	 * Triangle-triangle intersection test using Möller's algorithm
	 */
	static bool TrianglesIntersect(
		const FVector& V0, const FVector& V1, const FVector& V2,
		const FVector& U0, const FVector& U1, const FVector& U2)
	{
		// Compute plane equation of triangle 1
		const FVector N1 = FVector::CrossProduct(V1 - V0, V2 - V0);
		const float d1 = -FVector::DotProduct(N1, V0);

		// Check signed distances of triangle 2 vertices to plane of triangle 1
		const float du0 = FVector::DotProduct(N1, U0) + d1;
		const float du1 = FVector::DotProduct(N1, U1) + d1;
		const float du2 = FVector::DotProduct(N1, U2) + d1;

		// Early rejection if all points on the same side
		if ((du0 * du1 > 0.0f) && (du0 * du2 > 0.0f))
			return false;

		// Compute plane equation of triangle 2
		const FVector N2 = FVector::CrossProduct(U1 - U0, U2 - U0);
		const float d2 = -FVector::DotProduct(N2, U0);

		// Check signed distances of triangle 1 vertices to plane of triangle 2
		const float dv0 = FVector::DotProduct(N2, V0) + d2;
		const float dv1 = FVector::DotProduct(N2, V1) + d2;
		const float dv2 = FVector::DotProduct(N2, V2) + d2;

		// Early rejection if all points on the same side
		if ((dv0 * dv1 > 0.0f) && (dv0 * dv2 > 0.0f))
			return false;

		// Triangles are coplanar or intersecting
		return true;
	}
};

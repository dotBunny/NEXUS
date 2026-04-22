// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Geometric helpers for working with individual triangles in 3D space.
 *
 * All routines are header-only and allocation-free.
 */
class NEXUSCORE_API FNTriangleUtils
{
public:
	/**
	 * Tests whether Point lies inside the triangle (A, B, C) using barycentric coordinates.
	 * @param Point The 3D point to test. Must lie in (or very near) the triangle's plane for a meaningful result.
	 * @param A First triangle vertex.
	 * @param B Second triangle vertex.
	 * @param C Third triangle vertex.
	 * @return true when Point's barycentric coordinates are both non-negative and sum to no more than one.
	 */
	UE_FORCEINLINE_HINT static bool IsPointInTriangle(const FVector& Point, const FVector& A, const FVector& B, const FVector& C)
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
	 * Triangle-triangle intersection test using the full Möller 1997 algorithm.
	 *
	 * Performs plane-distance rejection, coplanar handling, and interval overlap on the intersection line.
	 * @param V0 First vertex of triangle one.
	 * @param V1 Second vertex of triangle one.
	 * @param V2 Third vertex of triangle one.
	 * @param U0 First vertex of triangle two.
	 * @param U1 Second vertex of triangle two.
	 * @param U2 Third vertex of triangle two.
	 * @return true when the two triangles intersect or are coplanar with an overlap.
	 */
	static bool TrianglesIntersect(const FVector& V0, const FVector& V1, const FVector& V2, const FVector& U0, const FVector& U1, const FVector& U2);

private:

	/**
	 * Computes the two scalar endpoints where triangle edges cross the intersection line.
	 * p0/p1/p2 are projections of the triangle vertices onto the intersection line.
	 * d0/d1/d2 are signed distances of those vertices to the opposing plane.
	 */
	UE_FORCEINLINE_HINT static void ComputeInterval(float p0, float p1, float p2, float d0, float d1, float d2, float& out0, float& out1)
	{
		if (d0 * d1 > 0.0f)
		{
			// d2 is the lone vertex; edges V2→V0 and V2→V1 cross the plane
			out0 = p2 + (p0 - p2) * (d2 / (d2 - d0));
			out1 = p2 + (p1 - p2) * (d2 / (d2 - d1));
		}
		else if (d0 * d2 > 0.0f)
		{
			// d1 is the lone vertex; edges V1→V0 and V1→V2 cross the plane
			out0 = p1 + (p0 - p1) * (d1 / (d1 - d0));
			out1 = p1 + (p2 - p1) * (d1 / (d1 - d2));
		}
		else
		{
			// d0 is the lone vertex (or degenerate); edges V0→V1 and V0→V2 cross the plane
			out0 = p0 + (p1 - p0) * (d0 / (d0 - d1));
			out1 = p0 + (p2 - p0) * (d0 / (d0 - d2));
		}
	}
};

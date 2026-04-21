// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Geometric helpers for working with individual triangles in 3D space.
 *
 * All routines are header-only and allocation-free.
 */
class FNTriangleUtils
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
	static bool TrianglesIntersect(
		const FVector& V0, const FVector& V1, const FVector& V2,
		const FVector& U0, const FVector& U1, const FVector& U2)
	{
		constexpr float Epsilon = 1e-6f;

		// Compute plane of T1: N1·X + d1 = 0
		const FVector N1 = FVector::CrossProduct(V1 - V0, V2 - V0);
		const float d1 = -FVector::DotProduct(N1, V0);

		// Signed distances of T2 verts to plane of T1
		float du0 = FVector::DotProduct(N1, U0) + d1;
		float du1 = FVector::DotProduct(N1, U1) + d1;
		float du2 = FVector::DotProduct(N1, U2) + d1;

		if (FMath::Abs(du0) < Epsilon) du0 = 0.0f;
		if (FMath::Abs(du1) < Epsilon) du1 = 0.0f;
		if (FMath::Abs(du2) < Epsilon) du2 = 0.0f;

		if ((du0 * du1 > 0.0f) && (du0 * du2 > 0.0f))
			return false;

		// Compute plane of T2: N2·X + d2 = 0
		const FVector N2 = FVector::CrossProduct(U1 - U0, U2 - U0);
		const float d2 = -FVector::DotProduct(N2, U0);

		// Signed distances of T1 verts to plane of T2
		float dv0 = FVector::DotProduct(N2, V0) + d2;
		float dv1 = FVector::DotProduct(N2, V1) + d2;
		float dv2 = FVector::DotProduct(N2, V2) + d2;

		if (FMath::Abs(dv0) < Epsilon) dv0 = 0.0f;
		if (FMath::Abs(dv1) < Epsilon) dv1 = 0.0f;
		if (FMath::Abs(dv2) < Epsilon) dv2 = 0.0f;

		if ((dv0 * dv1 > 0.0f) && (dv0 * dv2 > 0.0f))
			return false;

		// Direction of the intersection line of the two planes
		const FVector D = FVector::CrossProduct(N1, N2);

		// Coplanar case: triangles lie in the same plane
		if (D.SizeSquared() < Epsilon)
		{
			if (IsPointInTriangle(U0, V0, V1, V2)) return true;
			if (IsPointInTriangle(U1, V0, V1, V2)) return true;
			if (IsPointInTriangle(U2, V0, V1, V2)) return true;
			if (IsPointInTriangle(V0, U0, U1, U2)) return true;
			if (IsPointInTriangle(V1, U0, U1, U2)) return true;
			if (IsPointInTriangle(V2, U0, U1, U2)) return true;
			return false;
		}

		// Project onto the largest component of D for numerical stability
		int32 MaxComp = 0;
		if (FMath::Abs(D.Y) > FMath::Abs(D[MaxComp])) MaxComp = 1;
		if (FMath::Abs(D.Z) > FMath::Abs(D[MaxComp])) MaxComp = 2;

		const float pv0 = static_cast<float>(V0[MaxComp]);
		const float pv1 = static_cast<float>(V1[MaxComp]);
		const float pv2 = static_cast<float>(V2[MaxComp]);

		const float pu0 = static_cast<float>(U0[MaxComp]);
		const float pu1 = static_cast<float>(U1[MaxComp]);
		const float pu2 = static_cast<float>(U2[MaxComp]);

		float isect1_0, isect1_1;
		float isect2_0, isect2_1;
		ComputeInterval(pv0, pv1, pv2, dv0, dv1, dv2, isect1_0, isect1_1);
		ComputeInterval(pu0, pu1, pu2, du0, du1, du2, isect2_0, isect2_1);

		if (isect1_0 > isect1_1) Swap(isect1_0, isect1_1);
		if (isect2_0 > isect2_1) Swap(isect2_0, isect2_1);

		return !(isect1_1 < isect2_0 || isect2_1 < isect1_0);
	}

private:

	/**
	 * Computes the two scalar endpoints where triangle edges cross the intersection line.
	 * p0/p1/p2 are projections of the triangle vertices onto the intersection line.
	 * d0/d1/d2 are signed distances of those vertices to the opposing plane.
	 */
	static void ComputeInterval(float p0, float p1, float p2, float d0, float d1, float d2, float& out0, float& out1)
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

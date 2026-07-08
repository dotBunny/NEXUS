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

		// denom is a Gram determinant (dot00 * dot11 - dot01^2) and is therefore always >= 0. When it collapses
		// to (near) zero the triangle is degenerate/collinear and has no valid barycentric basis, so the point
		// cannot be inside. Testing deynom directly also catches tiny negative values from floating-point error.
		const float denom = dot00 * dot11 - dot01 * dot01;
		if (denom < SMALL_NUMBER)
		{
			return false;
		}

		// Compare the barycentric numerators against denom rather than dividing. Because denom > 0 here this is
		// algebraically identical to (u >= 0 && v >= 0 && u + v <= 1) but performs no division — which avoids
		// the false-positive C4723 (potential divide by 0) that MSVC raises when this header is inlined into a
		// unit test with degenerate constant vertices and constant-folds denom to 0.
		const float uNum = dot11 * dot02 - dot01 * dot12;
		const float vNum = dot00 * dot12 - dot01 * dot02;

		return (uNum >= 0.0f) && (vNum >= 0.0f) && (uNum + vNum <= denom);
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

	/**
	 * Distance from a point to the surface of a triangle.
	 *
	 * Finds the closest point on triangle (A, B, C) to P using a barycentric-region clamp
	 * (Ericson, Real-Time Collision Detection §5.1.5), then returns ||P - Closest||. Handles every
	 * Voronoi region of the triangle:
	 *   - perpendicular distance when P projects inside the face,
	 *   - edge-clamped distance when P projects past exactly one edge,
	 *   - vertex distance when P projects past two edges (a corner region).
	 *
	 * @param P The query point.
	 * @param A First triangle vertex.
	 * @param B Second triangle vertex.
	 * @param C Third triangle vertex.
	 * @return The minimum Euclidean distance from P to any point on the triangle's surface.
	 */
	UE_FORCEINLINE_HINT static double DistanceFromPointToTriangle(const FVector& P,
		const FVector& A, const FVector& B, const FVector& C)
	{
		const FVector AB = B - A;
		const FVector AC = C - A;
		const FVector AP = P - A;

		const double D1 = FVector::DotProduct(AB, AP);
		const double D2 = FVector::DotProduct(AC, AP);
		if (D1 <= 0.0 && D2 <= 0.0)
		{
			return FVector::Distance(P, A);
		}

		const FVector BP = P - B;
		const double D3 = FVector::DotProduct(AB, BP);
		const double D4 = FVector::DotProduct(AC, BP);
		if (D3 >= 0.0 && D4 <= D3)
		{
			return FVector::Distance(P, B);
		}

		const double VC = D1 * D4 - D3 * D2;
		if (VC <= 0.0 && D1 >= 0.0 && D3 <= 0.0)
		{
			const double DenomProtectionA = D1 - D3;
			if (FMath::Abs(DenomProtectionA) > UE_DOUBLE_SMALL_NUMBER)
			{
				const double V = D1 / DenomProtectionA;
				return FVector::Distance(P, A + AB * V);
			}
			return FVector::Distance(P, A);  // degenerate edge — snap to vertex
		}

		const FVector CP = P - C;
		const double D5 = FVector::DotProduct(AB, CP);
		const double D6 = FVector::DotProduct(AC, CP);
		if (D6 >= 0.0 && D5 <= D6)
		{
			return FVector::Distance(P, C);
		}

		const double VB = D5 * D2 - D1 * D6;
		if (VB <= 0.0 && D2 >= 0.0 && D6 <= 0.0)
		{
			const double DenomProtectionB = D2 - D6;
			if (FMath::Abs(DenomProtectionB) > UE_DOUBLE_SMALL_NUMBER)
			{
				const double W = D2 / DenomProtectionB;
				return FVector::Distance(P, A + AC * W);
			}
			return FVector::Distance(P, A);
		}

		const double VA = D3 * D6 - D5 * D4;
		if (VA <= 0.0 && (D4 - D3) >= 0.0 && (D5 - D6) >= 0.0)
		{
			const double W = (D4 - D3) / ((D4 - D3) + (D5 - D6));
			return FVector::Distance(P, B + (C - B) * W);
		}

		// P projects inside the triangle — distance is the perpendicular to the plane.
		const double Sum = VA + VB + VC;
		if (FMath::Abs(Sum) < UE_DOUBLE_SMALL_NUMBER) return FVector::Distance(P, A);
		const double DenomProtectionC = 1.0 / Sum;
		const double V = VB * DenomProtectionC;
		const double W = VC * DenomProtectionC;
		return FVector::Distance(P, A + AB * V + AC * W);
	}

	/**
	 * Möller-Trumbore ray-triangle intersection test.
	 *
	 * Returns true when the ray starting at Origin and travelling along Direction (does not need to be
	 * unit-length) pierces the interior of triangle (A, B, C) at strictly positive distance. The
	 * barycentric inequalities are strict (u > 0, v > 0, u + v < 1) so a ray that grazes an edge or
	 * vertex of the triangle is counted as a miss — combined with a non-axis-aligned Direction this
	 * keeps parity counts stable across an adjacent triangle pair (the shared edge is missed by both
	 * triangles, not double-counted on each).
	 *
	 * @param Origin Ray start position.
	 * @param Direction Ray direction (does not need to be normalised; only the sign of t is meaningful).
	 * @param A First triangle vertex.
	 * @param B Second triangle vertex.
	 * @param C Third triangle vertex.
	 * @param OutT Optional output for the ray parameter at the hit point (Origin + Direction * OutT).
	 * @return true when the ray crosses the triangle's interior at t > 0.
	 */
	UE_FORCEINLINE_HINT static bool RayIntersectsTriangle(const FVector& Origin, const FVector& Direction,
		const FVector& A, const FVector& B, const FVector& C, double* OutT = nullptr)
	{
		const FVector E1 = B - A;
		const FVector E2 = C - A;
		const FVector H = FVector::CrossProduct(Direction, E2);
		const double Det = FVector::DotProduct(E1, H);
		if (FMath::Abs(Det) < SMALL_NUMBER)
		{
			// Ray parallel to triangle plane (or triangle is degenerate).
			return false;
		}
		const double InvDet = 1.0 / Det;
		const FVector S = Origin - A;
		const double U = InvDet * FVector::DotProduct(S, H);
		if (U <= 0.0 || U >= 1.0)
		{
			return false;
		}
		const FVector Q = FVector::CrossProduct(S, E1);
		const double V = InvDet * FVector::DotProduct(Direction, Q);
		if (V <= 0.0 || U + V >= 1.0)
		{
			return false;
		}
		const double T = InvDet * FVector::DotProduct(E2, Q);
		if (T <= 0.0)
		{
			return false;
		}
		if (OutT != nullptr)
		{
			*OutT = T;
		}
		return true;
	}

private:

// #SONARQUBE-DISABLE-CPP_S107 Method required complexity
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
// #SONARQUBE-ENABLE-CPP_S107 Method required complexity
};

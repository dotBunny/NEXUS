// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NTriangleUtils.h"

// #SONARQUBE-DISABLE-CPP_S3776 Optimal complexity
bool FNTriangleUtils::TrianglesIntersect(const FVector& V0, const FVector& V1, const FVector& V2, const FVector& U0, const FVector& U1, const FVector& U2)
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
// #SONARQUBE-ENABLE-CPP_S3776 Optimal complexity
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Types/NRawMeshUtils.h"

#include "NCoreMinimal.h"
#include "Math/NTriangleUtils.h"
#include "Math/NVectorUtils.h"

bool FNRawMeshUtils::DoesIntersect(const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
                                   const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation)
{
	if (LeftMesh.Loops.Num() == 0 || RightMesh.Loops.Num() == 0)
	{
		UE_LOG(LogNexusCore, Warning, TEXT("No loops were found in the provided FNRawMeshes, unable to determine if there is any intersection."))
		return false;
	}
	if (LeftMesh.HasNonTris() || RightMesh.HasNonTris())
	{
		UE_LOG(LogNexusCore, Error, TEXT("One or both of the provided FNRawMeshes has non-triangle based geometry (NGons), this is not supported for intersection checks."));
		return false;
	}
	
	return DoesIntersectTriangles(LeftMesh, LeftOrigin, LeftRotation, RightMesh, RightOrigin, RightRotation);
}

bool FNRawMeshUtils::IsRelativePointInside(const FNRawMesh& Mesh, const FVector& RelativePoint)
{
	if (!Mesh.IsConvex() || Mesh.HasNonTris())
	{
		UE_LOG(LogNexusCore, Warning, TEXT("The FNRawMesh is either not convex or has non-triangles; unable to determine IsRelativePointInside; returning false."));
		return false;
	}
	
	for (const FNRawMeshLoop& Loop : Mesh.Loops)
	{
		const FVector& V0 = Mesh.Vertices[Loop.Indices[0]];
		const FVector& V1 = Mesh.Vertices[Loop.Indices[1]];
		const FVector& V2 = Mesh.Vertices[Loop.Indices[2]];
		
		const FVector Normal = FVector::CrossProduct(V1 - V0, V2 - V0);
		const float CenterSide = FVector::DotProduct(Normal, Mesh.Center - V0);
		const float PointSide  = FVector::DotProduct(Normal, RelativePoint - V0);
		
		if (CenterSide * PointSide < 0.0f)
		{
			return false;
		}
	}

	return true;
}

bool FNRawMeshUtils::AnyRelativePointsInside(const FNRawMesh& Mesh, const TArray<FVector>& RelativePoints)
{
	if (!Mesh.IsConvex() || Mesh.HasNonTris())
	{
		UE_LOG(LogNexusCore, Warning, TEXT("The FNRawMesh is either not convex or has non-triangles; unable to determine AnyRelativePointsInside; returning false."));
		return false;
	}

	for (const FVector& Point : RelativePoints)
	{
		if (IsRelativePointInside(Mesh, Point))
		{
			return true;
		}
	}
	return false;
}

bool FNRawMeshUtils::DoesIntersectTriangles(const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
                                            const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation)
{
	// Transform vertices of both meshes to world space
	TArray<FVector> LeftVerticesWorld;
	LeftVerticesWorld.Reserve(LeftMesh.Vertices.Num());
	for (const FVector& Vertex : LeftMesh.Vertices)
	{
		LeftVerticesWorld.Add(FNVectorUtils::TransformPoint(Vertex, LeftOrigin, LeftRotation));
	}

	TArray<FVector> RightVerticesWorld;
	RightVerticesWorld.Reserve(RightMesh.Vertices.Num());
	for (const FVector& Vertex : RightMesh.Vertices)
	{
		RightVerticesWorld.Add(FNVectorUtils::TransformPoint(Vertex, RightOrigin, RightRotation));
	}
	
	// Check every triangle against every other triangle
	const int32 LeftLoopCount = LeftMesh.Loops.Num();
	const int32 RightLoopCount = RightMesh.Loops.Num();

	for (int32 i = 0; i < LeftLoopCount; ++i)
	{
		const FVector& V0 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[0]];
		const FVector& V1 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[1]];
		const FVector& V2 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[2]];

		for (int32 j = 0; j < RightLoopCount; ++j)
		{
			const FVector& U0 = RightVerticesWorld[RightMesh.Loops[j].Indices[0]];
			const FVector& U1 = RightVerticesWorld[RightMesh.Loops[j].Indices[1]];
			const FVector& U2 = RightVerticesWorld[RightMesh.Loops[j].Indices[2]];

			if (FNTriangleUtils::TrianglesIntersect(V0, V1, V2, U0, U1, U2))
			{
				return true;
			}
		}
	}

	// Triangle-triangle tests only catch surface crossings. If one convex mesh is entirely
	// contained within the other, no triangles will intersect but the meshes still overlap.
	// Check containment by testing one vertex from each mesh against the other mesh.
	const FQuat LeftInvQuat = LeftRotation.Quaternion().Inverse();
	const FQuat RightInvQuat = RightRotation.Quaternion().Inverse();

	const FVector LeftSample = LeftInvQuat.RotateVector(LeftVerticesWorld[0] - RightOrigin);
	if (IsRelativePointInside(RightMesh, LeftSample))
	{
		return true;
	}

	const FVector RightSample = RightInvQuat.RotateVector(RightVerticesWorld[0] - LeftOrigin);
	if (IsRelativePointInside(LeftMesh, RightSample))
	{
		return true;
	}

	return false;
}
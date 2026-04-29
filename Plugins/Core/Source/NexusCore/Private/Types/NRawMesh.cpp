// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Types/NRawMesh.h"
#include "NCoreMinimal.h"
#include "BaseTools/BaseMeshProcessingTool.h"
#include "DynamicMesh/MeshNormals.h"

TArray<int32> FNRawMesh::GetFlatIndices()
{
	TArray<int32> ReturnData;
	const int32 LoopCount = Loops.Num();
	ReturnData.Reserve(LoopCount*3);
	for (int32 i = 0; i < LoopCount; i++)
	{
		for (int32 j = 0; j < Loops[i].Indices.Num(); j++)
		{
			ReturnData.Add(Loops[i].Indices[j]);
		}
	}
	return MoveTemp(ReturnData);
}

void FNRawMesh::ConvertToTriangles()
{
	// Early out
	if (!CheckNonTris())
	{
		return;
	}
	
	if (bIsConvex)
	{
		// Fan triangulation from vertex 0 — valid for convex loops only.
		for (int32 i = Loops.Num() - 1; i >= 0;  i--)
		{
			if (Loops[i].IsTriangle())
			{
				continue;
			}

			const TArray<int32> Src = Loops[i].Indices;
			Loops.RemoveAt(i);
			for (int32 j = Src.Num() - 2; j >= 1; j--)
			{
				Loops.Insert(FNRawMeshLoop(Src[0], Src[j], Src[j + 1]), i);
			}
		}
	}
	else
	{
		// Ear-clipping triangulation — handles concave loops by projecting each loop onto its
		// best-fit plane, then repeatedly clipping convex "ear" triangles that contain no other
		// loop vertices.
		for (int32 i = Loops.Num() - 1; i >= 0; i--)
		{
			if (Loops[i].IsTriangle())
			{
				continue;
			}

			const TArray<int32> Src = Loops[i].Indices;
			const int32 SrcCount = Src.Num();

			// Polygon normal via Newell's method — robust for slightly non-planar loops.
			FVector PolygonNormal = FVector::ZeroVector;
			for (int32 k = 0; k < SrcCount; k++)
			{
				const FVector& A = Vertices[Src[k]];
				const FVector& B = Vertices[Src[(k + 1) % SrcCount]];
				PolygonNormal.X += (A.Y - B.Y) * (A.Z + B.Z);
				PolygonNormal.Y += (A.Z - B.Z) * (A.X + B.X);
				PolygonNormal.Z += (A.X - B.X) * (A.Y + B.Y);
			}
			PolygonNormal = PolygonNormal.GetSafeNormal();

			// Build a 2D basis on the polygon's plane and project the loop into it.
			FVector Tangent;
			FVector Bitangent;
			PolygonNormal.FindBestAxisVectors(Tangent, Bitangent);

			TArray<FVector2D> Projected;
			Projected.Reserve(SrcCount);
			for (int32 k = 0; k < SrcCount; k++)
			{
				const FVector& V = Vertices[Src[k]];
				Projected.Emplace(FVector::DotProduct(V, Tangent), FVector::DotProduct(V, Bitangent));
			}

			// Signed area tells us the winding; traverse CCW so convex ears produce a positive cross.
			double SignedArea = 0.0;
			for (int32 k = 0; k < SrcCount; k++)
			{
				const FVector2D& A = Projected[k];
				const FVector2D& B = Projected[(k + 1) % SrcCount];
				SignedArea += (A.X * B.Y) - (B.X * A.Y);
			}

			TArray<int32> Ring;
			Ring.Reserve(SrcCount);
			if (SignedArea >= 0.0)
			{
				for (int32 k = 0; k < SrcCount; k++)
				{
					Ring.Add(k);
				}
			}
			else
			{
				for (int32 k = SrcCount - 1; k >= 0; k--)
				{
					Ring.Add(k);
				}
			}

			auto IsConvex = [&Projected](const int32 P, const int32 C, const int32 N)
			{
				const FVector2D& A = Projected[P];
				const FVector2D& B = Projected[C];
				const FVector2D& D = Projected[N];
				return ((B.X - A.X) * (D.Y - A.Y) - (B.Y - A.Y) * (D.X - A.X)) > 0.0;
			};

			auto PointInTri = [&Projected](const int32 PtIdx, const int32 A, const int32 B, const int32 C)
			{
				const FVector2D& Pt = Projected[PtIdx];
				const FVector2D& VA = Projected[A];
				const FVector2D& VB = Projected[B];
				const FVector2D& VC = Projected[C];
				const double D1 = (Pt.X - VB.X) * (VA.Y - VB.Y) - (VA.X - VB.X) * (Pt.Y - VB.Y);
				const double D2 = (Pt.X - VC.X) * (VB.Y - VC.Y) - (VB.X - VC.X) * (Pt.Y - VC.Y);
				const double D3 = (Pt.X - VA.X) * (VC.Y - VA.Y) - (VC.X - VA.X) * (Pt.Y - VA.Y);
				const bool bHasNeg = (D1 < 0.0) || (D2 < 0.0) || (D3 < 0.0);
				const bool bHasPos = (D1 > 0.0) || (D2 > 0.0) || (D3 > 0.0);
				return !(bHasNeg && bHasPos);
			};

			TArray<FNRawMeshLoop> NewTris;
			NewTris.Reserve(SrcCount - 2);

			// Guard against pathological polygons causing an infinite loop.
			int32 Guard = SrcCount * SrcCount;
			while (Ring.Num() > 3 && Guard-- > 0)
			{
				bool bEarFound = false;
				const int32 RingCount = Ring.Num();
				for (int32 k = 0; k < RingCount; k++)
				{
					const int32 Prev = Ring[(k + RingCount - 1) % RingCount];
					const int32 Curr = Ring[k];
					const int32 Next = Ring[(k + 1) % RingCount];

					if (!IsConvex(Prev, Curr, Next))
					{
						continue;
					}

					bool bContainsOther = false;
					for (int32 m = 0; m < RingCount; m++)
					{
						const int32 Idx = Ring[m];
						if (Idx != Prev && Idx != Curr && Idx != Next &&
							PointInTri(Idx, Prev, Curr, Next))
						{
							bContainsOther = true;
							break;
						}
					}
					if (bContainsOther)
					{
						continue;
					}

					NewTris.Emplace(Src[Prev], Src[Curr], Src[Next]);
					Ring.RemoveAt(k);
					bEarFound = true;
					break;
				}

				if (!bEarFound)
				{
					UE_LOG(LogNexusCore, Warning, TEXT("Ear-clipping failed on loop %d; falling back to fan of the remainder."), i);
					break;
				}
			}

			// Emit whatever remains (either the final triangle, or the untriangulated remainder as a fan).
			for (int32 k = 1; k < Ring.Num() - 1; k++)
			{
				NewTris.Emplace(Src[Ring[0]], Src[Ring[k]], Src[Ring[k + 1]]);
			}

			Loops.RemoveAt(i);
			for (int32 k = NewTris.Num() - 1; k >= 0; k--)
			{
				Loops.Insert(MoveTemp(NewTris[k]), i);
			}
		}
	}
}

FDynamicMesh3 FNRawMesh::CreateDynamicMesh(const bool bProcessMesh)
{
	if (CheckNonTris())
	{
		UE_LOG(LogNexusCore, Error, TEXT("The FNRawMesh contains non-triangular geometry; a FDynamicMesh must be triangulated."));
		return FDynamicMesh3();
	}

	// Add all of our vertices and indices for the triangles.
	// This doesn't mean it's a great mesh.
	FDynamicMesh3 DynamicMesh;
	DynamicMesh.BeginUnsafeVerticesInsert();
	{
		const int32 VertexCount = Vertices.Num();
		for (int32 i = 0; i < VertexCount; i++)
		{
			DynamicMesh.InsertVertex(i, Vertices[i], true);
		}
	}
	DynamicMesh.EndUnsafeVerticesInsert();
	
	DynamicMesh.BeginUnsafeTrianglesInsert();
	{
		const int32 LoopCount = Loops.Num();
		for (int32 i = 0; i < LoopCount; i++)
		{
			DynamicMesh.InsertTriangle(i,
				UE::Geometry::FIndex3i(
					Loops[i].Indices[0],
					Loops[i].Indices[1],
					Loops[i].Indices[2]));
		}
	}
	DynamicMesh.EndUnsafeTrianglesInsert();

	// So we need to process it
	if (bProcessMesh)
	{
		// Check and flip triangles if needed
		for (const int32 TriangleID : DynamicMesh.TriangleIndicesItr())
		{
			UE::Geometry::FIndex3i Triangle = DynamicMesh.GetTriangle(TriangleID);
			FVector V1 = DynamicMesh.GetVertex(Triangle.A);
			FVector V2 = DynamicMesh.GetVertex(Triangle.B);
			FVector V3 = DynamicMesh.GetVertex(Triangle.C);

			FVector Normal = FVector::CrossProduct(V2 - V1, V3 - V1).GetSafeNormal();
			FVector CenterToVertex = (V1 - Center).GetSafeNormal();

			// This should have been the other way?
			if (FVector::DotProduct(Normal, CenterToVertex) > 0)
			{
				DynamicMesh.ReverseTriOrientation(TriangleID);
			}
		}
    
		// Recompute normals after reorientation
		DynamicMesh.EnableVertexNormals(FVector3f::Zero());
		FMeshNormals::QuickComputeVertexNormals(DynamicMesh);
	}

	return MoveTemp(DynamicMesh);
}

bool FNRawMesh::CheckConvex()
{
	if (Vertices.Num() == 0 || Loops.Num() == 0)
	{
		UE_LOG(LogNexusCore, Warning, TEXT("No vertices or loops were found in the FNRawMesh when checking if it was convex."));
		return false;
	}

	// A mesh is convex if every loop is convex in its own plane AND every vertex of the mesh lies
	// on or behind every face's supporting plane. The first test catches flat concave n-gons; the
	// second catches concave polyhedra (e.g. a star).
	for (int32 LoopIdx = 0; LoopIdx < Loops.Num(); LoopIdx++)
	{
		const FNRawMeshLoop& Loop = Loops[LoopIdx];
		const int32 LoopCount = Loop.Indices.Num();
		if (LoopCount < 3)
		{
			continue;
		}

		// Face normal via Newell's method — works for triangles, quads, and n-gons (including slightly non-planar ones).
		FVector FaceNormal = FVector::ZeroVector;
		for (int32 k = 0; k < LoopCount; k++)
		{
			const FVector& A = Vertices[Loop.Indices[k]];
			const FVector& B = Vertices[Loop.Indices[(k + 1) % LoopCount]];
			FaceNormal.X += (A.Y - B.Y) * (A.Z + B.Z);
			FaceNormal.Y += (A.Z - B.Z) * (A.X + B.X);
			FaceNormal.Z += (A.X - B.X) * (A.Y + B.Y);
		}
		FaceNormal = FaceNormal.GetSafeNormal();
		if (FaceNormal.IsNearlyZero())
		{
			// Degenerate face — can't reason about its supporting plane.
			continue;
		}

		// In-plane convexity: every consecutive turn must keep the same sign relative to the face normal.
		for (int32 k = 0; k < LoopCount; k++)
		{
			const FVector& Prev = Vertices[Loop.Indices[(k + LoopCount - 1) % LoopCount]];
			const FVector& Curr = Vertices[Loop.Indices[k]];
			const FVector& Next = Vertices[Loop.Indices[(k + 1) % LoopCount]];
			const FVector Cross = FVector::CrossProduct(Curr - Prev, Next - Curr);
			if (FVector::DotProduct(Cross, FaceNormal) < -DBL_EPSILON)
			{
				return false;
			}
		}

		// Plane-side test: every vertex not on this face must lie on or behind its supporting plane.
		const FVector& FaceOrigin = Vertices[Loop.Indices[0]];
		const double PlaneD = FVector::DotProduct(FaceNormal, FaceOrigin);
		for (int32 v = 0; v < Vertices.Num(); v++)
		{
			if (Loop.Indices.Contains(v))
			{
				continue;
			}
			const double Signed = FVector::DotProduct(FaceNormal, Vertices[v]) - PlaneD;
			if (Signed > DBL_EPSILON)
			{
				return false;
			}
		}
	}

	return true;
}

bool FNRawMesh::CheckNonTris()
{
	const int32 LoopCount = Loops.Num();
	for (int32 i = 0; i < LoopCount; i++)
	{
		if (Loops[i].Indices.Num() != 3)
		{
			return true;
		}
	}
	return false;
}

bool FNRawMesh::CheckBounds() const
{
	return Bounds.IsValid != 0;
}

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Types/NRawMesh.h"
#include "NCoreMinimal.h"
#include "BaseTools/BaseMeshProcessingTool.h"
#include "DynamicMesh/MeshNormals.h"

TArray<int32> FNRawMesh::GetFlatIndices() const
{
	int32 Total = 0;
	for (const FNRawMeshLoop& Loop : Loops)
	{
		Total += Loop.Indices.Num();
	}

	TArray<int32> ReturnData;
	ReturnData.Reserve(Total);
	for (const FNRawMeshLoop& Loop : Loops)
	{
		ReturnData.Append(Loop.Indices);
	}
	return ReturnData;
}

void FNRawMesh::ConvertToTriangles()
{
	// Early out
	if (!CheckNonTris())
	{
		return;
	}

	// Build a fresh loop list and swap it in at the end — avoids O(K*N) shifting from RemoveAt/Insert
	// on the existing Loops array while we triangulate.
	TArray<FNRawMeshLoop> NewLoops;
	NewLoops.Reserve(Loops.Num());

	if (bIsConvex)
	{
		// Fan triangulation from vertex 0 — valid for convex loops only.
		for (const FNRawMeshLoop& Loop : Loops)
		{
			if (Loop.IsTriangle())
			{
				NewLoops.Add(Loop);
				continue;
			}

			const TArray<int32>& Src = Loop.Indices;
			for (int32 j = 1; j <= Src.Num() - 2; j++)
			{
				NewLoops.Emplace(Src[0], Src[j], Src[j + 1]);
			}
		}
	}
	else
	{
		// Ear-clipping triangulation — handles concave loops by projecting each loop onto its
		// best-fit plane, then repeatedly clipping convex "ear" triangles that contain no other
		// loop vertices.
		for (int32 LoopIdx = 0; LoopIdx < Loops.Num(); LoopIdx++)
		{
			const FNRawMeshLoop& Loop = Loops[LoopIdx];
			if (Loop.IsTriangle())
			{
				NewLoops.Add(Loop);
				continue;
			}

			const TArray<int32>& Src = Loop.Indices;
			const int32 SrcCount = Src.Num();

			const FVector PolygonNormal = ComputeLoopNormal(Src, Vertices);

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

			// Guard against pathological polygons causing an infinite loop.
			int32 Guard = SrcCount * SrcCount;
			while (Ring.Num() > 3 && Guard > 0)
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

					NewLoops.Emplace(Src[Prev], Src[Curr], Src[Next]);
					Ring.RemoveAt(k);
					bEarFound = true;
					break;
				}

				// Safety countdown
				Guard--;

				if (!bEarFound)
				{
					break;
				}
			}

			if (Ring.Num() == 3)
			{
				// Natural termination — the ring converged to a single triangle.
				NewLoops.Emplace(Src[Ring[0]], Src[Ring[1]], Src[Ring[2]]);
			}
			else
			{
				// Ear-clipping bailed with a concave remainder (or the guard expired). Fanning the
				// remainder would emit self-intersecting / zero-area triangles, so drop it instead —
				// missing geometry is recoverable; corrupt geometry silently breaks downstream physics
				// and rendering.
				UE_LOG(LogNexusCore, Warning,
					TEXT("Ear-clipping could not fully triangulate loop %d (%d unresolved vertices); dropping the remainder."),
					LoopIdx, Ring.Num());
			}
		}
	}

	Loops = MoveTemp(NewLoops);
}

void FNRawMesh::CalculateFaceLoops(const double AngleToleranceDeg, const double RelativeDistanceTolerance)
{
	FaceLoops.Reset();
	if (Loops.Num() == 0 || Vertices.Num() == 0)
	{
		return;
	}

	// Only operate on fully-triangulated input — anything else suggests FaceLoops already exists or the mesh isn't
	// in the shape this recovery algorithm understands.
	for (const FNRawMeshLoop& Loop : Loops)
	{
		if (Loop.Indices.Num() != 3)
		{
			return;
		}
	}

	const int32 NumTris = Loops.Num();

	// 1. Per-triangle Newell normals.
	TArray<FVector> Normals;
	Normals.SetNumUninitialized(NumTris);
	for (int32 i = 0; i < NumTris; i++)
	{
		const FVector& A = Vertices[Loops[i].Indices[0]];
		const FVector& B = Vertices[Loops[i].Indices[1]];
		const FVector& C = Vertices[Loops[i].Indices[2]];
		Normals[i] = FVector::CrossProduct(B - A, C - A).GetSafeNormal();
	}

	// 2. Edge → triangle map, keyed by sorted vertex pair so reverse-wound shared edges hash together.
	TMap<TPair<int32, int32>, TArray<int32, TInlineAllocator<2>>> EdgeToTris;
	EdgeToTris.Reserve(NumTris * 2);
	for (int32 TriIdx = 0; TriIdx < NumTris; TriIdx++)
	{
		const TArray<int32>& Idx = Loops[TriIdx].Indices;
		for (int32 k = 0; k < 3; k++)
		{
			const int32 V0 = Idx[k];
			const int32 V1 = Idx[(k + 1) % 3];
			const TPair<int32, int32> Key(FMath::Min(V0, V1), FMath::Max(V0, V1));
			EdgeToTris.FindOrAdd(Key).Add(TriIdx);
		}
	}

	// 3. Compute absolute coplanarity threshold from mesh extent so the tolerance scales with coordinate magnitude.
	FBox VertexBounds(ForceInit);
	for (const FVector& V : Vertices)
	{
		VertexBounds += V;
	}
	const double Extent = VertexBounds.IsValid != 0 ? VertexBounds.GetExtent().GetMax() * 2.0 : 1.0;
	const double DistThreshold = RelativeDistanceTolerance * Extent;
	const double CosThreshold = FMath::Cos(FMath::DegreesToRadians(AngleToleranceDeg));

	// 4. Union-find: merge triangles sharing an edge AND a plane (normal and offset within tolerance).
	TArray<int32> Parent;
	Parent.SetNumUninitialized(NumTris);
	for (int32 i = 0; i < NumTris; i++)
	{
		Parent[i] = i;
	}
	auto Find = [&Parent](int32 X) -> int32
	{
		while (Parent[X] != X)
		{
			Parent[X] = Parent[Parent[X]];
			X = Parent[X];
		}
		return X;
	};

	for (const auto& EdgeEntry : EdgeToTris)
	{
		const auto& Tris = EdgeEntry.Value;
		if (Tris.Num() != 2)
		{
			continue;
		}
		const int32 T0 = Tris[0];
		const int32 T1 = Tris[1];
		if (Normals[T0].IsNearlyZero() || Normals[T1].IsNearlyZero())
		{
			continue;
		}
		if (FVector::DotProduct(Normals[T0], Normals[T1]) < CosThreshold)
		{
			continue;
		}

		// Each T1 vertex must lie on T0's supporting plane within DistThreshold.
		const FVector& Origin0 = Vertices[Loops[T0].Indices[0]];
		const double Plane0D = FVector::DotProduct(Normals[T0], Origin0);
		bool bCoplanar = true;
		for (const int32 V : Loops[T1].Indices)
		{
			const double Signed = FVector::DotProduct(Normals[T0], Vertices[V]) - Plane0D;
			if (FMath::Abs(Signed) > DistThreshold)
			{
				bCoplanar = false;
				break;
			}
		}
		if (!bCoplanar)
		{
			continue;
		}

		const int32 R0 = Find(T0);
		const int32 R1 = Find(T1);
		if (R0 != R1)
		{
			Parent[R0] = R1;
		}
	}

	// 5. Bucket triangles by union-find root.
	TMap<int32, TArray<int32>> Groups;
	for (int32 i = 0; i < NumTris; i++)
	{
		Groups.FindOrAdd(Find(i)).Add(i);
	}

	// 6. For each group, walk the boundary directed edges to recover the polygon's winding.
	FaceLoops.Reserve(Groups.Num());
	for (auto& GroupEntry : Groups)
	{
		const TArray<int32>& GroupTris = GroupEntry.Value;
		if (GroupTris.Num() == 1)
		{
			FaceLoops.Add(Loops[GroupTris[0]]);
			continue;
		}

		TSet<int32> GroupSet;
		GroupSet.Reserve(GroupTris.Num());
		for (const int32 T : GroupTris)
		{
			GroupSet.Add(T);
		}

		// Boundary directed edges: a (V0, V1) edge in any group triangle is boundary iff no OTHER group triangle
		// contains the reverse edge (V1, V0). Internal edges between two coplanar siblings cancel out.
		TMap<int32, int32> NextVert;
		NextVert.Reserve(GroupTris.Num() * 3);
		for (const int32 T : GroupTris)
		{
			const TArray<int32>& Idx = Loops[T].Indices;
			for (int32 k = 0; k < 3; k++)
			{
				const int32 V0 = Idx[k];
				const int32 V1 = Idx[(k + 1) % 3];
				const TPair<int32, int32> Key(FMath::Min(V0, V1), FMath::Max(V0, V1));
				const auto* SharingTris = EdgeToTris.Find(Key);
				bool bInternal = false;
				if (SharingTris && SharingTris->Num() == 2)
				{
					const int32 Other = (*SharingTris)[0] == T ? (*SharingTris)[1] : (*SharingTris)[0];
					bInternal = GroupSet.Contains(Other);
				}
				if (!bInternal)
				{
					NextVert.Add(V0, V1);
				}
			}
		}

		// Walk a single closed boundary loop. Multi-loop groups (faces with holes) can't be represented as one
		// FNRawMeshLoop, so fall back to emitting the group's source triangles individually.
		bool bClosedLoop = false;
		TArray<int32> LoopIndices;
		if (NextVert.Num() > 0)
		{
			const int32 StartV = NextVert.CreateConstIterator().Key();
			LoopIndices.Reserve(NextVert.Num());
			int32 Curr = StartV;
			int32 Guard = NextVert.Num() + 1;
			while (Guard-- > 0)
			{
				LoopIndices.Add(Curr);
				const int32* Next = NextVert.Find(Curr);
				if (!Next)
				{
					break;
				}
				Curr = *Next;
				if (Curr == StartV)
				{
					bClosedLoop = (LoopIndices.Num() == NextVert.Num());
					break;
				}
			}
		}

		if (bClosedLoop && LoopIndices.Num() >= 3)
		{
			FaceLoops.Add(FNRawMeshLoop(MoveTemp(LoopIndices)));
		}
		else
		{
			for (const int32 T : GroupTris)
			{
				FaceLoops.Add(Loops[T]);
			}
		}
	}
}

FDynamicMesh3 FNRawMesh::CreateDynamicMesh(const bool bProcessMesh) const
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
		// Reorient any triangle whose normal faces inward (toward Center) so all faces face outward.
		for (const int32 TriangleID : DynamicMesh.TriangleIndicesItr())
		{
			const UE::Geometry::FIndex3i Triangle = DynamicMesh.GetTriangle(TriangleID);
			const FVector V1 = DynamicMesh.GetVertex(Triangle.A);
			const FVector V2 = DynamicMesh.GetVertex(Triangle.B);
			const FVector V3 = DynamicMesh.GetVertex(Triangle.C);

			const FVector Normal = FVector::CrossProduct(V2 - V1, V3 - V1).GetSafeNormal();
			const FVector Outward = (V1 - Center).GetSafeNormal();

			if (FVector::DotProduct(Normal, Outward) < 0.0)
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

bool FNRawMesh::CheckConvex() const
{
	// Prefer the polygonal face description when present — fan-triangulated coplanar faces will fail
	// the per-triangle plane-side test after any vertex perturbation, since each triangle's Newell
	// normal drifts independently and neighbor-triangle vertices end up infinitesimally off plane.
	const TArray<FNRawMeshLoop>& FacesToCheck = FaceLoops.Num() > 0 ? FaceLoops : Loops;

	if (Vertices.Num() == 0 || FacesToCheck.Num() == 0)
	{
		UE_LOG(LogNexusCore, Warning, TEXT("No vertices or loops were found in the FNRawMesh when checking if it was convex."));
		return false;
	}

	// A mesh is convex if every loop is convex in its own plane AND every vertex of the mesh lies
	// on or behind every face's supporting plane. The first test catches flat concave n-gons; the
	// second catches concave polyhedra (e.g. a star).
	for (int32 LoopIdx = 0; LoopIdx < FacesToCheck.Num(); LoopIdx++)
	{
		const FNRawMeshLoop& Loop = FacesToCheck[LoopIdx];
		const int32 LoopCount = Loop.Indices.Num();
		if (LoopCount < 3)
		{
			continue;
		}

		const FVector FaceNormal = ComputeLoopNormal(Loop.Indices, Vertices);
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
			if (FVector::DotProduct(Cross, FaceNormal) < -UE_DOUBLE_KINDA_SMALL_NUMBER)
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
			if (Signed > UE_DOUBLE_KINDA_SMALL_NUMBER)
			{
				return false;
			}
		}
	}

	return true;
}

bool FNRawMesh::CheckNonTris() const
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
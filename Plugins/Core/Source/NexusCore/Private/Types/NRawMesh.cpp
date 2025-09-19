﻿// Copyright dotBunny Inc. All Rights Reserved.
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
		for (int j = 0; j < Loops[i].Indices.Num(); j++)
		{
			ReturnData.Add(Loops[i].Indices[j]);
		}
	}
	return MoveTemp(ReturnData);
}

void FNRawMesh::ConvertToTriangles()
{
	// Early out
	if (!CheckQuads()) return;
	for (int i = Loops.Num() - 1; i >= 0;  i--)
	{
		if (Loops[i].IsQuad())
		{
			// We need to break apart the quad
			Loops.Insert(FNRawMeshLoop(Loops[i].Indices[2], Loops[i].Indices[3],Loops[i].Indices[0]), i+1 );

			// Remove extra
			Loops[i].Indices.RemoveAt(3);
		}
	}
}

FDynamicMesh3 FNRawMesh::CreateDynamicMesh(const bool bProcessMesh)
{
	if (CheckQuads())
	{
		N_LOG(Error, TEXT("[FNRawMesh::CreateDynamicMesh] Mesh contains quads, a FDynamicMesh cannot have quads. Please consider using FNRawMesh::ConvertToTriangles."));
		return nullptr;
	}

	// Add all of our vertices and indices for the triangles.
	// This doesn't mean it's a great mesh.
	FDynamicMesh3 DynamicMesh;
	DynamicMesh.BeginUnsafeVerticesInsert();
	{
		const int VertexCount = Vertices.Num();
		for (int i = 0; i < VertexCount; i++)
		{
			DynamicMesh.InsertVertex(i, Vertices[i], true);
		}
	}
	DynamicMesh.EndUnsafeVerticesInsert();
	
	DynamicMesh.BeginUnsafeTrianglesInsert();
	{
		const int LoopCount = Loops.Num();
		for (int i = 0; i < LoopCount; i++)
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
		// Calculate mesh center
		FVector Center = FVector::ZeroVector;
		for (int32 VertexID : DynamicMesh.VertexIndicesItr())
		{
			Center += DynamicMesh.GetVertex(VertexID);
		}
		Center /= DynamicMesh.VertexCount();

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
		return false;
	}

	FVector Center = FVector::ZeroVector;
	for (const FVector& Vertex : Vertices)
	{
		Center += Vertex;
	}
	Center /= Vertices.Num();
	// TODO: Do we want to actually cache the center position?

	for (int32 i = 0; i < Loops.Num(); i++)
	{
		if (Loops[i].Indices.Num() == 3)
		{
			FVector V1 = Vertices[Loops[i].Indices[0]];
			FVector V2 = Vertices[Loops[i].Indices[1]];
			FVector V3 = Vertices[Loops[i].Indices[2]];

			FVector Normal = FVector::CrossProduct(V2 - V1, V3 - V1).GetSafeNormal();
			FVector CenterToVertex = (V1 - Center).GetSafeNormal();

			if (FVector::DotProduct(Normal, CenterToVertex) < 0)
			{
				return false;
			}
		}
		else if (Loops[i].Indices.Num() == 4)
		{
			FVector V1 = Vertices[Loops[i].Indices[0]];
			FVector V2 = Vertices[Loops[i].Indices[1]];
			FVector V3 = Vertices[Loops[i].Indices[2]];
			FVector V4 = Vertices[Loops[i].Indices[3]];

			FVector NormalA = FVector::CrossProduct(V2 - V1, V3 - V1).GetSafeNormal();
			FVector NormalB = FVector::CrossProduct(V3 - V2, V4 - V2).GetSafeNormal();

			FVector CenterToVertexA = (V1 - Center).GetSafeNormal();
			FVector CenterToVertexB = (V2 - Center).GetSafeNormal();

			if (FVector::DotProduct(NormalA, CenterToVertexA) < 0 ||
				FVector::DotProduct(NormalB, CenterToVertexB) < 0)
			{
				return false;
			}
		}
		else
		{
			N_LOG(Warning, TEXT("[FNRawMesh::CheckConvex] Loop %d is not a triangle or quad, cannot determine convexity."), i);
			return false;
		}
	}
	return true;
}
bool FNRawMesh::CheckQuads()
{
	const int LoopCount = Loops.Num();
	for (int i = 0; i < LoopCount; i++)
	{
		if (Loops[i].IsQuad())
		{
			return true;
		}
	}
	return false;
}

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NVolumeUtils.h"

#include "DynamicMeshBuilder.h"

void FNVolumeUtils::FillGeometryData(UModel* Model, FNVolumeGeometryData& OutData)
{
    if (!Model) return;

    OutData.Indices.Empty();
    OutData.Vertices.Empty();

    int32 VertexOffset = 0;
    for (int32 NodeIndex = 0; NodeIndex < Model->Nodes.Num(); NodeIndex++)
    {
        const FBspNode& Node = Model->Nodes[NodeIndex];
        if (!Model->Surfs.IsValidIndex(Node.iSurf)) continue;

        const FBspSurf& Surf = Model->Surfs[Node.iSurf];
        if (!Model->Vectors.IsValidIndex(Surf.vTextureU) ||
            !Model->Vectors.IsValidIndex(Surf.vTextureV) ||
            !Model->Vectors.IsValidIndex(Surf.vNormal))
        {
            continue;
        }

        const int32 NumVerticesInNode = Node.NumVertices;

        // Validate the full vertex range up front so a corrupt entry can't leave a partial
        // node behind that would desynchronise the index buffer below.
        bool bVertsValid = true;
        for (int32 i = 0; i < NumVerticesInNode; i++)
        {
            const int32 VertIndex = Node.iVertPool + i;
            if (!Model->Verts.IsValidIndex(VertIndex) ||
                !Model->Points.IsValidIndex(Model->Verts[VertIndex].pVertex))
            {
                bVertsValid = false;
                break;
            }
        }
        if (!bVertsValid) continue;

        // The vectors used to calculate UVs and Normals
        FVector TextureU = FVector(Model->Vectors[Surf.vTextureU]);
        FVector TextureV = FVector(Model->Vectors[Surf.vTextureV]);
        FVector Normal = FVector(Model->Vectors[Surf.vNormal]);
        FVector TangentX = TextureU ^ Normal;
        TangentX.Normalize();

        // Add Vertex
        for (int32 i = 0; i < NumVerticesInNode; i++)
        {
            const FVert& BspVert = Model->Verts[Node.iVertPool + i];
            FVector Position = static_cast<FVector>(Model->Points[BspVert.pVertex]);

            FDynamicMeshVertex DynVert;
            DynVert.SetTangents(FVector3f(TangentX), FVector3f(TextureV ^ Normal), FVector3f(Normal));
            DynVert.TextureCoordinate[0] = FVector2f(Position | TextureU, Position | TextureV);
            DynVert.Position = FVector3f(Position);
            DynVert.Color = FColor::White;
            OutData.Vertices.Add(DynVert);
        }

        // Add Indices
        for (int32 i = 2; i < NumVerticesInNode; i++)
        {
            OutData.Indices.Add(VertexOffset);
            OutData.Indices.Add(VertexOffset + i - 1);
            OutData.Indices.Add(VertexOffset + i);
        }

        VertexOffset += NumVerticesInNode;
    }
}

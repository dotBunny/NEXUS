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
        const FBspSurf& Surf = Model->Surfs[Node.iSurf];
        
        const int32 NumVerticesInNode = Node.NumVertices;
        
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

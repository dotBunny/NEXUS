// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Cell/NCellJunctionDetails.h"
#include "Cell/NCellVoxelData.h"
#include "Math/NVectorUtils.h"

struct FNRawMesh;

class NEXUSPROCGEN_API FNProcGenDebugDraw
{
public:

	static void DrawSocket(FPrimitiveDrawInterface* PDI, const FVector& Location, const FRotator& Rotation, 
		const FIntVector2& UnitSize, const FVector2D& SocketSize, const ENCellJunctionType& SocketType, const FLinearColor& Color);
	
	static void DrawDashedRawMesh(FPrimitiveDrawInterface* PDI, const FNRawMesh& Mesh, const FRotator& Rotation, const FVector& Offset,
		FLinearColor Color, float DashSize = 2, ESceneDepthPriorityGroup Priority = SDPG_World);

	static void DrawDashedRawMesh(FPrimitiveDrawInterface* PDI, const FNRawMesh& Mesh, const TArray<FVector>& WorldVertices,
		FLinearColor Color, float DashSize = 2, ESceneDepthPriorityGroup Priority = SDPG_World);
	
	static void DrawVoxelDataGrid(FPrimitiveDrawInterface* PDI, const FNCellVoxelData& VoxelData, const FVector& Offset, const FRotator& Rotation);
	static void DrawVoxelDataPoints(FPrimitiveDrawInterface* PDI, const FNCellVoxelData& VoxelData, const FVector& Offset, const FRotator& Rotation);
};
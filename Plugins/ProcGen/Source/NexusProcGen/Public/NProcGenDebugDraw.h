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
	/**
	 * Draws a rectangular representation of the provided socket, rotated 90-degrees as to better represent the socket.
	 * @param PDI Drawing Interface
	 * @param Location The World Location that is the center of the drawn rectangle
	 * @param Rotation The World Rotation that represents the forward direction of the socket.
	 * @param UnitSize A unit representation of size (width(x) and height(y)). No concept of depth.
	 * @param SocketSize The actual world size of each unit dimension (width(x) and height(y)).
	 * @param SocketType An indicator of the socket type determining what sort of additional information is drawn.
	 * @param Color The color to draw the socket.
	 */
	static void DrawSocket(FPrimitiveDrawInterface* PDI, const FVector& Location, const FRotator& Rotation, 
	                       const FIntVector2& UnitSize, const FVector2D& SocketSize, const ENCellJunctionType& SocketType, const FLinearColor& Color);
	
	static void DrawDashedRawMesh(FPrimitiveDrawInterface* PDI, const FNRawMesh& Mesh, const FRotator& Rotation, const FVector& Offset,
		FLinearColor Color, float DashSize = 2, ESceneDepthPriorityGroup Priority = SDPG_World);

	static void DrawDashedRawMesh(FPrimitiveDrawInterface* PDI, const FNRawMesh& Mesh, const TArray<FVector>& WorldVertices,
		FLinearColor Color, float DashSize = 2, ESceneDepthPriorityGroup Priority = SDPG_World);
	
	static void DrawVoxelDataGrid(FPrimitiveDrawInterface* PDI, const FNCellVoxelData& VoxelData, const FVector& Offset, const FRotator& Rotation);
	static void DrawVoxelDataPoints(FPrimitiveDrawInterface* PDI, const FNCellVoxelData& VoxelData, const FVector& Offset, const FRotator& Rotation);
};
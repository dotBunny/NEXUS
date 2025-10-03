// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Cell/NCellJunctionDetails.h"
#include "Math/NVectorUtils.h"

#define PDI_LINE_THICKNESS 1.5f

struct FNRawMesh;

class NEXUSPROCGEN_API FNProcGenDebugDraw
{
public:
	
	static void DrawJunctionUnits(FPrimitiveDrawInterface* PDI, const FVector& WorldCenter, const FRotator& Rotation,
		const TArray<FVector2D>& Points, FLinearColor Color,  const float Radius = 10.0f,
		const ENAxis Axis = ENAxis::Z, ESceneDepthPriorityGroup Priority = SDPG_Foreground);	

	static void DrawJunctionRectangle(FPrimitiveDrawInterface* PDI, const TArray<FVector>& Points, FLinearColor Color,
		const ENAxis Axis = ENAxis::Z, ESceneDepthPriorityGroup Priority = SDPG_Foreground);

	static void DrawJunctionSocketTypePoint(FPrimitiveDrawInterface* PDI, const FVector& Location, const FRotator& Rotation, const ENCellJunctionType& Type, const float Length);
	
	static void DrawDashedRawMesh(FPrimitiveDrawInterface* PDI, const FNRawMesh& Mesh, const FRotator& Rotation, const FVector& Offset,
		FLinearColor Color, float DashSize = 2, ESceneDepthPriorityGroup Priority = SDPG_World);

	static void DrawDashedRawMesh(FPrimitiveDrawInterface* PDI, const FNRawMesh& Mesh, const TArray<FVector>& WorldVertices,
		FLinearColor Color, float DashSize = 2, ESceneDepthPriorityGroup Priority = SDPG_World);
};
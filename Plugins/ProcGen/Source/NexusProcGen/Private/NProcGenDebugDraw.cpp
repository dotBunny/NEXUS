// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenDebugDraw.h"

#include "NProcGenUtils.h"
#include "Math/NVectorUtils.h"
#include "Types/NRawMesh.h"

void FNProcGenDebugDraw::DrawJunctionUnits(FPrimitiveDrawInterface* PDI, const FVector& WorldCenter,
	const FRotator& Rotation, const TArray<FVector2D>& Points, const FLinearColor& Color, const float Radius,  const ENAxis Axis,
	const ESceneDepthPriorityGroup Priority)
{
	
	const int PointCount = Points.Num();
	for (int i = 0; i < PointCount; i++)
	{
		FVector Location = FNVectorUtils::RotatedAroundPivot(WorldCenter + FVector(Points[i].X, 0.0f, Points[i].Y), WorldCenter, Rotation);
		switch (Axis)
		{
		case X:
			DrawCircle(PDI, Location, FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y), FRotationMatrix(Rotation).GetScaledAxis(EAxis::X), Color, Radius, 32, Priority, PDI_LINE_THICKNESS);
			break;
		case Y:
			DrawCircle(PDI, Location, FRotationMatrix(Rotation).GetScaledAxis(EAxis::X), FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y), Color, Radius, 32, Priority, PDI_LINE_THICKNESS);
			break;
		default:
		case Z:
			DrawCircle(PDI, Location, FRotationMatrix(Rotation).GetScaledAxis(EAxis::X), FRotationMatrix(Rotation).GetScaledAxis(EAxis::Z), Color, Radius, 32, Priority, PDI_LINE_THICKNESS);
			break;
		}
	}
}

void FNProcGenDebugDraw::DrawJunctionRectangle(FPrimitiveDrawInterface* PDI, const TArray<FVector>& Points,
	const FLinearColor& Color, const ENAxis Axis, const ESceneDepthPriorityGroup Priority)
{
	PDI->AddReserveLines(SDPG_World, 4, false, false);
	
	PDI->DrawLine(Points[1], Points[2], Color, Priority, PDI_LINE_THICKNESS);
	PDI->DrawLine(Points[2], Points[3], Color, Priority, PDI_LINE_THICKNESS);
	PDI->DrawLine(Points[3], Points[0], Color, Priority, PDI_LINE_THICKNESS);
	PDI->DrawLine(Points[0], Points[1], Color, Priority, PDI_LINE_THICKNESS);
}

void FNProcGenDebugDraw::DrawJunctionSocketTypePoint(FPrimitiveDrawInterface* PDI, const FVector& Location,
	const FRotator& Rotation, const FLinearColor& Color, const ENCellJunctionType& Type, const float Length)
{
	switch (Type)
	{
	case ENCellJunctionType::CJT_TwoWaySocket:
		const FVector TwoWayPointA = Location + (Rotation.Vector() * Length);
		const FVector TwoWayPointB = Location + (Rotation.Vector() * -Length);
		PDI->DrawLine(TwoWayPointA, TwoWayPointB, Color, SDPG_Foreground, PDI_LINE_THICKNESS);
		break;
	case ENCellJunctionType::CJT_OneWaySocket:
		const FVector OneWayPoint = Location + (Rotation.Vector() * Length);
		PDI->DrawLine(Location, OneWayPoint, Color, SDPG_Foreground, PDI_LINE_THICKNESS);
		break;
	}
}

void FNProcGenDebugDraw::DrawDashedRawMesh(FPrimitiveDrawInterface* PDI, const FNRawMesh& Mesh, const FRotator& Rotation, const FVector& Offset, const FLinearColor Color, const float DashSize, const ESceneDepthPriorityGroup Priority)
{
	const TArray<FVector> WorldVertices = FNVectorUtils::RotateAndOffsetPoints(Mesh.Vertices, Rotation, Offset);
	DrawDashedRawMesh(PDI, Mesh, WorldVertices, Color, DashSize, Priority);
}

void FNProcGenDebugDraw::DrawDashedRawMesh(FPrimitiveDrawInterface* PDI, const FNRawMesh& Mesh, const TArray<FVector>& WorldVertices, const FLinearColor Color, const float DashSize, const ESceneDepthPriorityGroup Priority)
{
	const TArray<FNRawMeshLoop>& Loops = Mesh.Loops;
	const int LoopCount = Loops.Num();

	for (int i = 0; i < LoopCount; i++)
	{
		const FNRawMeshLoop& Loop = Loops[i];
		const int Stride = Loop.Indices.Num();
		const int StrideShort = Stride - 1;
		
		for (int j = 0; j < StrideShort; j++)
		{
			DrawDashedLine(PDI, WorldVertices[Loop.Indices[j]] ,WorldVertices[Loop.Indices[j+1]], Color, DashSize, Priority);
		}
		DrawDashedLine(PDI, WorldVertices[Loop.Indices[StrideShort]] ,WorldVertices[Loop.Indices[0]], Color, DashSize, Priority);
	}
}

void FNProcGenDebugDraw::DrawVoxelDataGrid(FPrimitiveDrawInterface* PDI, const FNCellVoxelData& VoxelData, const FVector& Offset, const FRotator& Rotation)
{
	// TODO: Optimized draw process
	
}

void FNProcGenDebugDraw::DrawVoxelDataPoints(FPrimitiveDrawInterface* PDI, const FNCellVoxelData& VoxelData, const FVector& Offset, const FRotator& Rotation)
{
	// Draw Points toggle?
}

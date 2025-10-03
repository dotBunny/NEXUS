// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenDebugDraw.h"

#include "Math/NVectorUtils.h"
#include "Types/NRawMesh.h"

void FNProcGenDebugDraw::DrawJunctionUnits(FPrimitiveDrawInterface* PDI, const FVector& WorldCenter,
	const FRotator& Rotation, const TArray<FVector2D>& Points, FLinearColor Color, const float Radius,  const ENAxis Axis,
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

void FNProcGenDebugDraw::DrawJunctionRectangle(FPrimitiveDrawInterface* PDI, const FVector& WorldCenter, const FRotator& Rotation,
                                               const float Width, const float Height, const FLinearColor Color, const ENAxis Axis, const ESceneDepthPriorityGroup Priority)
{
	const float HalfWidth = Width * 0.5f;
	const float HalfHeight = Height * 0.5f;

	// Common case UP
	FVector TopLeft = FVector(-HalfWidth,0,HalfHeight);
	FVector BottomLeft = FVector(-HalfWidth,0,-HalfHeight);
	FVector BottomRight = FVector(HalfWidth,0,-HalfHeight);
	FVector TopRight = FVector(HalfWidth,0,HalfHeight);

	if (Axis == ENAxis::X)
	{
		TopLeft = FVector(HalfHeight, 0, -HalfWidth);
		BottomLeft = FVector(-HalfHeight, 0, -HalfWidth);
		BottomRight = FVector(-HalfHeight, 0, HalfWidth);
		TopRight = FVector(HalfHeight, 0, HalfWidth);
	}
	else if (Axis == ENAxis::Y)
	{
		TopLeft = FVector(HalfWidth,HalfHeight,0);
		BottomLeft = FVector(HalfWidth,-HalfHeight,0);
		BottomRight = FVector(-HalfWidth,-HalfHeight,0);
		TopRight = FVector(-HalfWidth,HalfHeight, 0);
	}
	
	// Rotate the points around the center
	TopLeft = FNVectorUtils::RotatedAroundPivot(WorldCenter + TopLeft, WorldCenter, Rotation);
	BottomLeft = FNVectorUtils::RotatedAroundPivot(WorldCenter + BottomLeft, WorldCenter, Rotation);
	BottomRight = FNVectorUtils::RotatedAroundPivot(WorldCenter + BottomRight, WorldCenter, Rotation);
	TopRight = FNVectorUtils::RotatedAroundPivot(WorldCenter + TopRight, WorldCenter, Rotation);
	
	PDI->AddReserveLines(SDPG_World, 4, false, false);
	PDI->DrawLine(BottomLeft, BottomRight, Color, Priority, PDI_LINE_THICKNESS);
	PDI->DrawLine(BottomRight, TopRight, Color, Priority, PDI_LINE_THICKNESS);
	PDI->DrawLine(TopRight, TopLeft, Color, Priority, PDI_LINE_THICKNESS);
	PDI->DrawLine(TopLeft, BottomLeft, Color, Priority, PDI_LINE_THICKNESS);
}

void FNProcGenDebugDraw::DrawDashedRawMesh(FPrimitiveDrawInterface* PDI, const FNRawMesh& Mesh, const FRotator& Rotation, const FVector& Offset, const FLinearColor Color, const float DashSize, const ESceneDepthPriorityGroup Priority)
{
	const TArray<FVector> WorldVertices = FNVectorUtils::RotateAndOffsetVectors(Mesh.Vertices, Rotation, Offset);
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



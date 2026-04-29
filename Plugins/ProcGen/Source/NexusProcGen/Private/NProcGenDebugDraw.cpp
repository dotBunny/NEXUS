// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenDebugDraw.h"

#include "NProcGenMinimal.h"
#include "NProcGenSettings.h"
#include "NProcGenUtils.h"
#include "Math/NVectorUtils.h"
#include "Types/NRawMesh.h"

void FNProcGenDebugDraw::DrawSocket(FPrimitiveDrawInterface* PDI, const FVector& Location, const FRotator& Rotation, 
	const FIntVector2& UnitSize, const FVector2D& SocketSize, const ENCellJunctionType& SocketType, const FLinearColor& Color)
{
	// Compose: yaw-align the local XZ-plane corners into the YZ-plane (rect normal becomes +X),
	// then apply the socket rotation. Rotator addition is component-wise and only matches
	// composition for yaw-only inputs; quaternions compose correctly for pitch and roll too.
	const FQuat DisplayQuat = FQuat(Rotation) * FQuat(FRotator(0.0f, 90.0f, 0.0f));
	const FRotator DisplayRotation = DisplayQuat.Rotator();
	const FVector FacingRotation = Rotation.Vector();

	const FVector2D Size = FNProcGenUtils::GetWorldSize2D(UnitSize, SocketSize);
	const float LineLength = SocketSize.X * 0.25f;

	const TArray<FVector> UnrotatedCornerPoints = FNProcGenUtils::GetCenteredWorldCornerPoints2D(Size.X,Size.Y, ENAxis::Z);
	const TArray<FVector> RotatedCornerPoints = FNVectorUtils::RotateAndOffsetPoints(UnrotatedCornerPoints, DisplayRotation, Location);
	const TArray<FVector2D> UnrotatedSocketPoints = FNProcGenUtils::GetSocketPoints2D(UnitSize, SocketSize);
	const int32 SocketPointsCount = UnrotatedSocketPoints.Num();
	
	const FVector DirectionEndPoint = Location + (FacingRotation * 42.f);
	const FVector DirectionTop = Location + (FVector::UpVector * 5.f);
	const FVector DirectionTopPoint = DirectionTop + (FacingRotation * 35.f);
	
	const FVector DirectionBottom = Location - (FVector::UpVector * 5.f);
	const FVector DirectionBottomPoint = DirectionBottom + (FacingRotation * 35.f);
	
	// Estimate of lines: Rectangle(4) + Corner Lines(4) + Direction Arrow(3)
	const int32 ReserveLineCount = 11 + (SocketPointsCount*32);
	PDI->AddReserveLines(SDPG_Foreground, ReserveLineCount, false, false);
	
	// Draw Rectangle
	PDI->DrawLine(RotatedCornerPoints[1], RotatedCornerPoints[2], Color, SDPG_Foreground, NEXUS::ProcGen::Debug::LineThickness);
	PDI->DrawLine(RotatedCornerPoints[2], RotatedCornerPoints[3], Color, SDPG_Foreground, NEXUS::ProcGen::Debug::LineThickness);
	PDI->DrawLine(RotatedCornerPoints[3], RotatedCornerPoints[0], Color, SDPG_Foreground, NEXUS::ProcGen::Debug::LineThickness);
	PDI->DrawLine(RotatedCornerPoints[0], RotatedCornerPoints[1], Color, SDPG_Foreground, NEXUS::ProcGen::Debug::LineThickness);
	
	// Draw Socket Points
	for (int32 i = 0; i < SocketPointsCount; i++)
	{
		FVector RotatedNubPoint = FNVectorUtils::RotatedAroundPivot(Location + 
			FVector(UnrotatedSocketPoints[i].X, 0.0f, UnrotatedSocketPoints[i].Y), Location, DisplayRotation);
		
		DrawCircle(PDI, RotatedNubPoint, 
			FRotationMatrix(DisplayRotation).GetScaledAxis(EAxis::X), FRotationMatrix(DisplayRotation).GetScaledAxis(EAxis::Z), 
			Color, 10.f, 32, SDPG_Foreground, NEXUS::ProcGen::Debug::LineThickness);
	}
	
	// Draw Corner Lines
	for (int32 i = 0; i < 4; i++)
	{
		switch (SocketType)
		{
			using enum ENCellJunctionType;
		case TwoWaySocket:
			const FVector TwoWayPointA = RotatedCornerPoints[i] + (FacingRotation * LineLength);
			const FVector TwoWayPointB = RotatedCornerPoints[i] + (FacingRotation * -LineLength);
			PDI->DrawLine(TwoWayPointA, TwoWayPointB, Color, SDPG_Foreground, NEXUS::ProcGen::Debug::LineThickness);
			break;
		
		case InOnlySocket:
			const FVector InOnlySocketPoint = RotatedCornerPoints[i] + (FacingRotation * -LineLength);
			PDI->DrawLine(RotatedCornerPoints[i], InOnlySocketPoint, Color, SDPG_Foreground, NEXUS::ProcGen::Debug::LineThickness);
			break;	
		
		case OutOnlySocket:
			const FVector OutOnlySocketPoint = RotatedCornerPoints[i] + (FacingRotation * LineLength);
			PDI->DrawLine(RotatedCornerPoints[i], OutOnlySocketPoint, Color, SDPG_Foreground, NEXUS::ProcGen::Debug::LineThickness);
			break;		

		case OneWaySocket:
			break;
		}
	}
	
	// Draw Direction
	PDI->DrawLine(Location, DirectionEndPoint, Color, SDPG_Foreground, NEXUS::ProcGen::Debug::LineThickness);
	PDI->DrawLine(DirectionEndPoint, DirectionTopPoint, Color, SDPG_Foreground, NEXUS::ProcGen::Debug::LineThickness);
	PDI->DrawLine(DirectionEndPoint, DirectionBottomPoint, Color, SDPG_Foreground, NEXUS::ProcGen::Debug::LineThickness);
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

	for (int32 i = 0; i < LoopCount; i++)
	{
		const FNRawMeshLoop& Loop = Loops[i];
		const int32 Stride = Loop.Indices.Num();
		const int32 StrideShort = Stride - 1;
		
		for (int32 j = 0; j < StrideShort; j++)
		{
			DrawDashedLine(PDI, WorldVertices[Loop.Indices[j]] ,WorldVertices[Loop.Indices[j+1]], Color, DashSize, Priority);
		}
		DrawDashedLine(PDI, WorldVertices[Loop.Indices[StrideShort]] ,WorldVertices[Loop.Indices[0]], Color, DashSize, Priority);
	}
}

void FNProcGenDebugDraw::DrawVoxelDataGrid(FPrimitiveDrawInterface* PDI, const FNCellVoxelData& VoxelData, const FVector& Offset, const FRotator& Rotation)
{
	const size_t PointCount = VoxelData.GetCount();
	if (PointCount == 0) return;
	
	const UNProcGenSettings* Settings = GetDefault<UNProcGenSettings>();
	const FVector UnitSize = Settings->VoxelSize;
	const FVector HalfUnitSize = UnitSize * 0.5f;
	const FVector BaseOffset = VoxelData.Origin + Offset;
		
	for (int32 i = 0; i < PointCount; i++)
	{
		auto [x,y,z] = VoxelData.GetInverseIndex(i);
		const FVector VoxelCenter = BaseOffset + ((FVector(x, y, z) * UnitSize) + HalfUnitSize);
			
		// TODO: #ROTATE-VOXELS Rotation needs to actually rotated to the nearest grid???
		const FVector VoxelMin = VoxelCenter - HalfUnitSize;
		const FVector VoxelMax = VoxelCenter + HalfUnitSize;
		
		if (N_FLAGS_HAS(VoxelData.GetData(i), static_cast<uint8>(ENCellVoxel::Occupied)))
		{
			
			DrawWireBox(PDI, FBox(VoxelMin, VoxelMax), FColor::Blue, SDPG_World );
		}
	}
}

void FNProcGenDebugDraw::DrawVoxelDataPoints(FPrimitiveDrawInterface* PDI, const FNCellVoxelData& VoxelData, const FVector& Offset, const FRotator& Rotation)
{
	if (!VoxelData.IsValid()) return;
	
	const size_t PointCount = VoxelData.GetCount();
	if (PointCount == 0) return;
	
	const UNProcGenSettings* Settings = GetDefault<UNProcGenSettings>();
	const FVector UnitSize = Settings->VoxelSize;
	const FVector HalfUnitSize = UnitSize * 0.5f;
	const FVector BaseOffset = VoxelData.Origin + Offset;
	
	for (int32 i = 0; i < PointCount; i++)
	{
		auto [x,y,z] = VoxelData.GetInverseIndex(i);
		
		// TODO: #ROTATE-VOXELS Rotation needs to actually rotated to the nearest grid???
		FVector VoxelCenter = BaseOffset + ((FVector(x, y, z) * UnitSize) + HalfUnitSize);

		// Represent origin as a box
		if (x == 0 && y == 0 && z == 0)
		{
			const FVector VoxelMin = VoxelCenter - HalfUnitSize;
			const FVector VoxelMax = VoxelCenter + HalfUnitSize;
			DrawWireBox(PDI, FBox(VoxelMin, VoxelMax), FColor::Yellow, SDPG_Foreground );
		}

		if (N_FLAGS_HAS(VoxelData.GetData(i), static_cast<uint8>(ENCellVoxel::Occupied)))
		{
			PDI->DrawPoint(VoxelCenter, FColor::Blue, 5.f, SDPG_Foreground);
		}
	}
}

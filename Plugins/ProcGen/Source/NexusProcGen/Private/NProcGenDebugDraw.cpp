// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenDebugDraw.h"

#include "NProcGenMinimal.h"
#include "NProcGenSettings.h"
#include "Math/NVectorUtils.h"
#include "Types/NRawMesh.h"

// #SONARQUBE-DISABLE-S107 Verbosity necessary.
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
			using enum ENAxis;
		case X:
			DrawCircle(PDI, Location, FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y), FRotationMatrix(Rotation).GetScaledAxis(EAxis::X), Color, Radius, 32, Priority, NEXUS::ProcGen::Debug::LineThickness);
			break;
		case Y:
			DrawCircle(PDI, Location, FRotationMatrix(Rotation).GetScaledAxis(EAxis::X), FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y), Color, Radius, 32, Priority, NEXUS::ProcGen::Debug::LineThickness);
			break;
		case Z:
		default:
			DrawCircle(PDI, Location, FRotationMatrix(Rotation).GetScaledAxis(EAxis::X), FRotationMatrix(Rotation).GetScaledAxis(EAxis::Z), Color, Radius, 32, Priority, NEXUS::ProcGen::Debug::LineThickness);
			break;
		}
	}
}
// #SONARQUBE-ENABLE

void FNProcGenDebugDraw::DrawJunctionRectangle(FPrimitiveDrawInterface* PDI, const TArray<FVector>& Points,
	const FLinearColor& Color, const ENAxis Axis, const ESceneDepthPriorityGroup Priority)
{
	PDI->AddReserveLines(SDPG_World, 4, false, false);
	
	PDI->DrawLine(Points[1], Points[2], Color, Priority, NEXUS::ProcGen::Debug::LineThickness);
	PDI->DrawLine(Points[2], Points[3], Color, Priority, NEXUS::ProcGen::Debug::LineThickness);
	PDI->DrawLine(Points[3], Points[0], Color, Priority, NEXUS::ProcGen::Debug::LineThickness);
	PDI->DrawLine(Points[0], Points[1], Color, Priority, NEXUS::ProcGen::Debug::LineThickness);
}

void FNProcGenDebugDraw::DrawJunctionSocketTypePoint(FPrimitiveDrawInterface* PDI, const FVector& Location,
	const FRotator& Rotation, const FLinearColor& Color, const ENCellJunctionType& Type, const float Length)
{
	switch (Type)
	{
	case ENCellJunctionType::TwoWaySocket:
		const FVector TwoWayPointA = Location + (Rotation.Vector() * Length);
		const FVector TwoWayPointB = Location + (Rotation.Vector() * -Length);
		PDI->DrawLine(TwoWayPointA, TwoWayPointB, Color, SDPG_Foreground, NEXUS::ProcGen::Debug::LineThickness);
		break;
	case ENCellJunctionType::OneWaySocket:
		const FVector OneWayPoint = Location + (Rotation.Vector() * Length);
		PDI->DrawLine(Location, OneWayPoint, Color, SDPG_Foreground, NEXUS::ProcGen::Debug::LineThickness);
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
	const size_t PointCount = VoxelData.GetCount();
	if (PointCount == 0) return;
	
	const UNProcGenSettings* Settings = GetDefault<UNProcGenSettings>();
	const FVector UnitSize = Settings->UnitSize;
	const FVector HalfUnitSize = UnitSize * 0.5f;
	const FVector BaseOffset = VoxelData.Origin + Offset;
	
	for (int i = 0; i < PointCount; i++)
	{
		auto [x,y,z] = VoxelData.GetInverseIndex(i);
		const FVector VoxelCenter = BaseOffset + ((FVector(x, y, z) * UnitSize) + HalfUnitSize);
			
		// TODO: #ROTATE-VOXELS Rotation needs to actually rotated to the nearest grid???
		const FVector VoxelMin = VoxelCenter - HalfUnitSize;
		const FVector VoxelMax = VoxelCenter + HalfUnitSize;
		
		if (N_FLAGS_HAS(VoxelData.GetData(i), static_cast<uint8>(ENCellVoxel::Occupied)))
		{
			DrawWireBox(PDI, FBox(VoxelMin, VoxelMax), FColor::Blue, SDPG_Foreground );
		}
		else
		{
			DrawWireBox(PDI, FBox(VoxelMin, VoxelMax), FColor::Green, SDPG_Foreground );
		}
	}
}

void FNProcGenDebugDraw::DrawVoxelDataPoints(FPrimitiveDrawInterface* PDI, const FNCellVoxelData& VoxelData, const FVector& Offset, const FRotator& Rotation)
{
	if (!VoxelData.IsValid()) return;
	
	const size_t PointCount = VoxelData.GetCount();
	if (PointCount == 0) return;
	
	const UNProcGenSettings* Settings = GetDefault<UNProcGenSettings>();
	const FVector UnitSize = Settings->UnitSize;
	const FVector HalfUnitSize = UnitSize * 0.5f;
	const FVector BaseOffset = VoxelData.Origin + Offset;
	
	for (int i = 0; i < PointCount; i++)
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
		else
		{
			PDI->DrawPoint(VoxelCenter, FColor::Green, 5.f, SDPG_Foreground);
		}
	}
}

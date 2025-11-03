// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenUtils.h"
#include "NArrayUtils.h"
#include "Organ/NOrganVolume.h"
#include "Chaos/Convex.h"
#include "Organ/NOrganComponent.h"


FBox FNProcGenUtils::CalculatePlayableBounds(ULevel* InLevel, const FNCellBoundsGenerationSettings& Settings)
{
	FBox LevelBounds(ForceInit);
	if (InLevel)
	{
		for (int32 ActorIndex = 0; ActorIndex < InLevel->Actors.Num() ; ++ActorIndex)
		{
			const AActor* Actor = InLevel->Actors[ActorIndex];
			if (Actor && Actor->IsLevelBoundsRelevant())
			{
				// Ignore Tags
				if (FNArrayUtils::ContainsAny(Actor->Tags, Settings.ActorIgnoreTags)) continue;
				
				const FBox ActorBox = Actor->GetComponentsBoundingBox(Settings.bIncludeNonColliding);
				if (ActorBox.IsValid)
				{
					LevelBounds+= ActorBox;
				}
			}
		}
	}

	return LevelBounds;
}


FNRawMesh FNProcGenUtils::CalculateConvexHull(ULevel* InLevel, const FNCellHullGenerationSettings& Settings)
{
	FNRawMesh Mesh;
	TArray<Chaos::FConvex::FVec3Type> Vertices;
	
	if (InLevel)
	{
		FVector BoxVertices[8];
		const int32 NumActors = InLevel->Actors.Num();
		Vertices.Reserve(NumActors * 8);
		for (int32 ActorIndex = 0; ActorIndex < InLevel->Actors.Num() ; ++ActorIndex)
		{
			const AActor* Actor = InLevel->Actors[ActorIndex];
			if (Actor && Actor->IsLevelBoundsRelevant())
			{
				// Ignore Tags
				if (FNArrayUtils::ContainsAny(Actor->Tags, Settings.ActorIgnoreTags)) continue;
				
				FBox ActorBox = Actor->GetComponentsBoundingBox(Settings.bIncludeNonColliding);
				if (ActorBox.IsValid)
				{
					if (ActorBox.GetExtent().X > 0 && ActorBox.GetExtent().Y > 0 && ActorBox.GetExtent().Z > 0)
					{
						ActorBox.GetVertices(BoxVertices);

						Vertices.Add(Chaos::FConvex::FVec3Type(BoxVertices[0]));
						Vertices.Add(Chaos::FConvex::FVec3Type(BoxVertices[1]));
						Vertices.Add(Chaos::FConvex::FVec3Type(BoxVertices[2]));
						Vertices.Add(Chaos::FConvex::FVec3Type(BoxVertices[3]));
						Vertices.Add(Chaos::FConvex::FVec3Type(BoxVertices[4]));
						Vertices.Add(Chaos::FConvex::FVec3Type(BoxVertices[5]));
						Vertices.Add(Chaos::FConvex::FVec3Type(BoxVertices[6]));
						Vertices.Add(Chaos::FConvex::FVec3Type(BoxVertices[7]));
					}
				}
			}
		}
	}
	
	TArray<Chaos::FConvex::FPlaneType> OutPlanes;
	TArray<TArray<int32>> OutFaceIndices;
	TArray<Chaos::FConvex::FVec3Type> OutVertices;
	Chaos::FConvex::FAABB3Type OutLocalBounds;

	Chaos::FConvexBuilder::FConvexBuilder::Build(Vertices, OutPlanes, OutFaceIndices, OutVertices, OutLocalBounds, Settings.GetChaosBuildMethod());
	
	// Construct FVector Vertices
	const int VerticesCount = OutVertices.Num();
	Mesh.Vertices.Reserve(VerticesCount);
	for (int i = 0; i < VerticesCount; i++)
	{
		Mesh.Vertices.Add(FVector(OutVertices[i][0], OutVertices[i][1], OutVertices[i][2]));
	}

	// Build Loops
	const int IndicesCount = OutFaceIndices.Num();
	Mesh.Loops.Reserve(IndicesCount);
	for (int i = 0; i < IndicesCount; i++)
	{
		// Right now we are 
		Mesh.Loops.Add(FNRawMeshLoop(OutFaceIndices[i]));	
	}
	
	// Mark as a convex mesh
	Mesh.bIsConvex = true;
	Mesh.ConvertToTriangles();
	return Mesh;
}

FNCellVoxelData FNProcGenUtils::CalculateVoxelData(ULevel* InLevel, const FNCellVoxelGenerationSettings& Settings)
{
	FNCellVoxelData ReturnData;
	
	// TODO: Calculate voxel data for the actor?
	
	ReturnData.Resize(10,10, 10);
	ReturnData.SetData(0,0,0, static_cast<uint8>(ENCellVoxel::CVD_Occupied));
	ReturnData.SetData(9,9,9, static_cast<uint8>(ENCellVoxel::CVD_Occupied));
	
	return MoveTemp(ReturnData);
}

int FNProcGenUtils::GetCellActorCountFromLevel(const ULevel* Level)
{
	int Count = 0;
	for (auto ActorIt = Level->Actors.CreateConstIterator(); ActorIt; ++ActorIt )
	{
		if (ActorIt->IsA<ANCellActor>())
		{
			Count++;
		}
	}
	return Count;
}

int FNProcGenUtils::GetCellActorCountFromWorld(const UWorld* World, const bool bIgnoreInstancedLevels)
{
	int Count = 0;
	for (const ULevel* Level : World->GetLevels())
	{
		if (bIgnoreInstancedLevels && Level->IsInstancedLevel()) continue;
		for (auto ActorIt = Level->Actors.CreateConstIterator(); ActorIt; ++ActorIt )
		{
			if (ActorIt->IsA<ANCellActor>())
			{
				Count++;
			}
		}
	}
	return Count;
}

ANCellActor* FNProcGenUtils::GetCellActorFromLevel(const ULevel* Level)
{
	if (Level == nullptr) return nullptr;
	for (auto ActorIt = Level->Actors.CreateConstIterator(); ActorIt; ++ActorIt )
	{
		if (ActorIt->IsA<ANCellActor>())
		{
			return Cast<ANCellActor>(ActorIt->Get());
		}
	}
	return nullptr;
}

ANCellActor* FNProcGenUtils::GetCellActorFromWorld(const UWorld* World, const bool bIgnoreInstancedLevels)
{
	if (World == nullptr ) return nullptr;
	for (const ULevel* Level : World->GetLevels())
	{
		if (bIgnoreInstancedLevels && Level->IsInstancedLevel()) continue;
		for (auto ActorIt = Level->Actors.CreateConstIterator(); ActorIt; ++ActorIt )
		{
			if (ActorIt->IsA<ANCellActor>())
			{
				return Cast<ANCellActor>(ActorIt->Get());
			}
		}
	}
	return nullptr;
}

TArray<UNOrganComponent*> FNProcGenUtils::GetOrganComponentsFromLevel(const ULevel* InLevel)
{
	TArray<UNOrganComponent*> Result;
	if (InLevel == nullptr) return Result;
    
	for (auto ActorIt = InLevel->Actors.CreateConstIterator(); ActorIt; ++ActorIt)
	{
		if (const AActor* Actor = ActorIt->Get())
		{
			TArray<UNOrganComponent*> Components;
			Actor->GetComponents<UNOrganComponent>(Components);
			Result.Append(Components);
		}
	}
    
	return Result;

}

TArray<ANOrganVolume*> FNProcGenUtils::GetOrganVolumesFromLevel(const ULevel* InLevel)
{
	TArray<ANOrganVolume*> Result;
	if (InLevel == nullptr) return Result;
	for (auto ActorIt = InLevel->Actors.CreateConstIterator(); ActorIt; ++ActorIt )
	{
		if (ActorIt->IsA<ANOrganVolume>())
		{
			Result.Add(Cast<ANOrganVolume>(ActorIt->Get()));
		}
	}
	return Result;
}

TArray<ANOrganVolume*> FNProcGenUtils::GetOrganVolumesFromWorld(const UWorld* World, bool bIgnoreInstancedLevels)
{
	TArray<ANOrganVolume*> Result;
	if (World == nullptr ) return Result;
	
	for (const ULevel* Level : World->GetLevels())
	{
		if (bIgnoreInstancedLevels && Level->IsInstancedLevel()) continue;
		for (auto ActorIt = Level->Actors.CreateConstIterator(); ActorIt; ++ActorIt )
		{
			if (ActorIt->IsA<ANOrganVolume>())
			{
				Result.Add(Cast<ANOrganVolume>(ActorIt->Get()));
			}
		}
	}
	return Result;
}

TArray<FVector2D> FNProcGenUtils::GetCenteredWorldPoints2D(const FIntVector2& Units, const FVector& UnitSize)
{
	TArray<FVector2D> Points;
	Points.Reserve( Units.X * Units.Y);
	const double UsableWidth = ((Units.X * UnitSize.X) - UnitSize.X);
	const double UsableHeight = ((Units.Y * UnitSize.Y) - UnitSize.Y);
	const double WidthStart = -(UsableWidth * 0.5f);
	const double HeightStart = -(UsableHeight * 0.5f);

	for (int i = 0; i < Units.X; i++)
	{
		for ( int y = 0; y < Units.Y; y++)
		{
			Points.Add(FVector2D(WidthStart + (i * UnitSize.X), HeightStart + (y * UnitSize.Y)));
		}
	}
	return MoveTemp(Points);
}

TArray<FVector> FNProcGenUtils::GetCenteredWorldCornerPoints2D(const FVector& WorldCenter, const FRotator& Rotation,
	const float Width, const float Height, const ENAxis Axis)
{
	const float HalfWidth = Width * 0.5f;
	const float HalfHeight = Height * 0.5f;

	TArray<FVector> ReturnPositions;
	ReturnPositions.Reserve(4);

	// Common case UP
	ReturnPositions.Add(FVector(-HalfWidth,0,HalfHeight)); // TL
	ReturnPositions.Add(FVector(-HalfWidth,0,-HalfHeight)); // BL
	ReturnPositions.Add(FVector(HalfWidth,0,-HalfHeight)); // BR
	ReturnPositions.Add(FVector(HalfWidth,0,HalfHeight)); // TR
	
	if (Axis == ENAxis::X)
	{
		ReturnPositions[0] = FVector(HalfHeight, 0, -HalfWidth);
		ReturnPositions[1] = FVector(-HalfHeight, 0, -HalfWidth);
		ReturnPositions[2] = FVector(-HalfHeight, 0, HalfWidth);
		ReturnPositions[3] = FVector(HalfHeight, 0, HalfWidth);
	}
	else if (Axis == ENAxis::Y)
	{
		ReturnPositions[0] = FVector(HalfWidth,HalfHeight,0);
		ReturnPositions[1] = FVector(HalfWidth,-HalfHeight,0);
		ReturnPositions[2] = FVector(-HalfWidth,-HalfHeight,0);
		ReturnPositions[3] = FVector(-HalfWidth,HalfHeight, 0);
	}
	
	ReturnPositions[0] = FNVectorUtils::RotatedAroundPivot(WorldCenter + ReturnPositions[0], WorldCenter, Rotation);
	ReturnPositions[1] = FNVectorUtils::RotatedAroundPivot(WorldCenter + ReturnPositions[1], WorldCenter, Rotation);
	ReturnPositions[2] = FNVectorUtils::RotatedAroundPivot(WorldCenter + ReturnPositions[2], WorldCenter, Rotation);
	ReturnPositions[3] = FNVectorUtils::RotatedAroundPivot(WorldCenter + ReturnPositions[3], WorldCenter, Rotation);

	return MoveTemp(ReturnPositions);
}


// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenUtils.h"

#include "NArrayUtils.h"
#include "NProcGenSettings.h"
#include "Cell/NCellJunctionComponent.h"
#include "Organ/NOrganVolume.h"
#include "Chaos/Convex.h"
#include "Organ/NOrganComponent.h"
#include "Misc/ScopedSlowTask.h"

FBox FNProcGenUtils::CalculatePlayableBounds(ULevel* InLevel, const FNCellBoundsGenerationSettings& Settings)
{
	FBox LevelBounds(ForceInit);
	
	// Go home early
	if (InLevel == nullptr)
	{
		return MoveTemp(LevelBounds);
	}
	
	const int32 NumActors = InLevel->Actors.Num();
	FScopedSlowTask Task = FScopedSlowTask(NumActors, NSLOCTEXT("NexusProcGen", "Task_CalculatePlayableBounds", "Calculate Playable Bounds"));
	Task.MakeDialog(false);

	for (int32 ActorIndex = 0; ActorIndex < NumActors; ++ActorIndex)
	{
		
		const AActor* Actor = InLevel->Actors[ActorIndex];
		Task.EnterProgressFrame(1);
		
		if (Actor && Actor->IsLevelBoundsRelevant())
		{
			// Check Editor Only
			if (Actor->IsEditorOnly() && !Settings.bIncludeEditorOnly) continue;
			
			// Ignore Tags
			if (FNArrayUtils::ContainsAny(Actor->Tags, Settings.ActorIgnoreTags)) continue;
			
			const FBox ActorBox = Actor->GetComponentsBoundingBox(Settings.bIncludeNonColliding);
			if (ActorBox.IsValid)
			{
				LevelBounds+= ActorBox;
			}
		}
	}
	
	return MoveTemp(LevelBounds);
}


FNRawMesh FNProcGenUtils::CalculateConvexHull(ULevel* InLevel, const FNCellHullGenerationSettings& Settings)
{
	FNRawMesh Mesh;
	TArray<Chaos::FConvex::FVec3Type> Vertices;
	
	if (InLevel)
	{
		
		FVector BoxVertices[8];
		const int32 NumActors = InLevel->Actors.Num();
		
		FScopedSlowTask ActorTask = FScopedSlowTask(NumActors, NSLOCTEXT("NexusProcGen", "Task_CalculateConvexHull_Actor", "Calculate Convex Hull - Actors"));
		ActorTask.MakeDialog(false);
		
		Vertices.Reserve(NumActors * 8);
		for (int32 ActorIndex = 0; ActorIndex < InLevel->Actors.Num() ; ++ActorIndex)
		{
			ActorTask.EnterProgressFrame(1);
			const AActor* Actor = InLevel->Actors[ActorIndex];
			if (Actor && Actor->IsLevelBoundsRelevant())
			{
				// Check Editor Only
				if (Actor->IsEditorOnly() && !Settings.bIncludeEditorOnly) continue;
				
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

	FScopedSlowTask ChaosTask = FScopedSlowTask(2, NSLOCTEXT("NexusProcGen", "Task_CalculateConvexHull_Chaos", "Calculate Convex Hull - Chaos"));
	ChaosTask.MakeDialog(false);
	ChaosTask.EnterProgressFrame(1);
	Chaos::FConvexBuilder::FConvexBuilder::Build(Vertices, OutPlanes, OutFaceIndices, OutVertices, OutLocalBounds, Settings.GetChaosBuildMethod());
	ChaosTask.EnterProgressFrame(1);
	
	
	
	// Construct FVector Vertices
	const int VerticesCount = OutVertices.Num();
	const int IndicesCount = OutFaceIndices.Num();
	
	FScopedSlowTask BuildTask = FScopedSlowTask(VerticesCount + IndicesCount, NSLOCTEXT("NexusProcGen", "Task_CalculateConvexHull_Build", "Calculate Convex Hull - Build Mesh"));
	BuildTask.MakeDialog(false);
	
	Mesh.Vertices.Reserve(VerticesCount);
	for (int i = 0; i < VerticesCount; i++)
	{
		BuildTask.EnterProgressFrame(1);
		Mesh.Vertices.Add(FVector(OutVertices[i][0], OutVertices[i][1], OutVertices[i][2]));
	}

	// Build Loops
	
	Mesh.Loops.Reserve(IndicesCount);
	for (int i = 0; i < IndicesCount; i++)
	{
		BuildTask.EnterProgressFrame(1);
		
		// Right now we are 
		Mesh.Loops.Add(FNRawMeshLoop(OutFaceIndices[i]));	
	}
	
	// Because the generator will have processed the mesh and even though it could have ngons it is still a convex hull
	Mesh.bIsChaosGenerated = true;
	Mesh.bIsConvex = true;
	Mesh.bHasNonTris = Mesh.CheckNonTris();
	
	return Mesh;
}

FNCellVoxelData FNProcGenUtils::CalculateVoxelData(ULevel* InLevel, const FNCellVoxelGenerationSettings& Settings)
{
	// TODO: We probably could use the voxel data to actually generate the overall bounds to avoid the double parse of the actors in the level
	FNCellVoxelData ReturnData;
	
	// Go home early
	if (InLevel == nullptr)
	{
		return MoveTemp(ReturnData);
	}

	// Settings
	const UWorld* World = InLevel->GetWorld();
	const FVector UnitSize = UNProcGenSettings::Get()->UnitSize;
	const ECollisionChannel CollisionChannel = Settings.CollisionChannel;
	const FVector HalfUnitSize = UnitSize * 0.5f;
	TArray<const AActor*> IgnoredActors;
	
	
	// STEP 1 - Specific Bounds / Ignore Actors
	FBox Bounds(ForceInit);
	const int32 NumActors = InLevel->Actors.Num();
	FScopedSlowTask BoundsTask = FScopedSlowTask(NumActors, NSLOCTEXT("NexusProcGen", "Task_CalculateVoxelData_Bounds", "Build Voxel World"));
	BoundsTask.MakeDialog(false);
	for (int32 ActorIndex = 0; ActorIndex < NumActors; ++ActorIndex)
	{
		const AActor* Actor = InLevel->Actors[ActorIndex];
		BoundsTask.EnterProgressFrame(1);

		if (Actor && Actor->IsLevelBoundsRelevant())
		{
			// Ignore Tags
			if (FNArrayUtils::ContainsAny(Actor->Tags, Settings.ActorIgnoreTags))
			{
				IgnoredActors.Add(Actor);
				continue;
			}
			
			// Check Editor Only
			if (Actor->IsEditorOnly() && !Settings.bIncludeEditorOnly)
			{
				IgnoredActors.Add(Actor);
				continue;
			}
				
			const FBox ActorBox = Actor->GetComponentsBoundingBox(Settings.bIncludeNonColliding);
			if (ActorBox.IsValid)
			{
				Bounds+= ActorBox;
			}
			
			
		}
	}
	
	// Determine the Voxel origin adjustment
	ReturnData.Origin = Bounds.Min;
	
	const FBox UnitBounds = FBox(
				FNVectorUtils::GetFurthestGridIntersection(Bounds.Min, UnitSize),
				FNVectorUtils::GetFurthestGridIntersection(Bounds.Max, UnitSize));
	
	const int64 SizeX = FMath::RoundToInt(FMath::Abs(UnitBounds.Min.X) + FMath::Abs(UnitBounds.Max.X));	
	const int64 SizeY = FMath::RoundToInt(FMath::Abs(UnitBounds.Min.Y) + FMath::Abs(UnitBounds.Max.Y));	
	const int64 SizeZ = FMath::RoundToInt(FMath::Abs(UnitBounds.Min.Z) + FMath::Abs(UnitBounds.Max.Z));	
	
	// Setup array
	ReturnData.Resize(SizeX, SizeY, SizeZ);
	const size_t Count = ReturnData.GetCount();
	
	FCollisionQueryParams Params = FCollisionQueryParams(TEXT("CalculateVoxelData"), true);
	Params.AddIgnoredActors(IgnoredActors);
	
	// STEP 2 - Broad Trace
	FScopedSlowTask BroadTraceTask = FScopedSlowTask(Count, NSLOCTEXT("NexusProcGen", "Task_CalculateVoxelData_BroadTrace", "Broad Trace"));
	BroadTraceTask.MakeDialog(false);

	// We iterate over the array by axis to minimize inverse calculations
	TArray<FVector> RayEndPoints;
	FHitResult SingleHit;
	TArray<FHitResult> ObjectHits;
	
	// Our initial box shape is slightly larger than the actual voxel unit size as to always detect collisions right on the extents.
	FCollisionShape BoxShape = FCollisionShape::MakeBox(HalfUnitSize + FVector(0.001f, 0.001f, 0.001f));
	TArray<uint32> SurroundingIndices;
	
	// #SONARQUBE-DISABLE
	// Need to loop depth to handle dimensions
	for (int x = 0; x < SizeX; x++)
	{
		for (int y = 0; y < SizeY; y++)
		{
			for (int z = 0; z < SizeZ; z++)
			{
				const size_t VoxelIndex = ReturnData.GetIndex(x,y,z);
				BroadTraceTask.EnterProgressFrame(1);
				FVector VoxelCenter = ReturnData.Origin + ((FVector(x, y, z) * UnitSize) + HalfUnitSize);
				
				// Standard Overlap Check
				bool const bHit = World ? World->SweepSingleByChannel(SingleHit, VoxelCenter, VoxelCenter, FQuat::Identity, CollisionChannel, BoxShape, Params) : false;
				if (bHit)
				{
					ReturnData.AddFlag(VoxelIndex, ENCellVoxel::CVD_Occupied);
				}
			}
		}
	}
	// #SONARQUBE-ENABLE
	
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

void FNProcGenUtils::GetVoxelQueryPoints(const FVector& WorldCenter, const FVector& VoxelSize, TArray<FVector>& OutPositions)
{
	/** Directional Vectors (26)
	 TOP			MIDDLE				BOTTOM
	 0 - 1 - 2		9 -  10 -  11		17 -  18 -  19
	 |       |		|           |		|           |
	 3   8   4		12    X    13		20    25    21
	 |       |		|		    |		|		    |
	 5 - 6 - 7		14 - 15 -  16		22 -  23 -  24
	 */
	
	OutPositions.Add(WorldCenter + FVector(VoxelSize.X,-VoxelSize.Y, VoxelSize.Z)); // TOP_0
	OutPositions.Add(WorldCenter + FVector(VoxelSize.X,0, VoxelSize.Z)); // TOP_1
	OutPositions.Add(WorldCenter + FVector(VoxelSize.X, VoxelSize.Y, VoxelSize.Z)); // TOP_2
	OutPositions.Add(WorldCenter + FVector(0,-VoxelSize.Y, VoxelSize.Z)); // TOP_3
	OutPositions.Add(WorldCenter + FVector(0,0, VoxelSize.Z)); // TOP_8
	OutPositions.Add(WorldCenter + FVector(0,VoxelSize.Y, VoxelSize.Z)); // TOP_4
	OutPositions.Add(WorldCenter + FVector(-VoxelSize.X,-VoxelSize.Y, VoxelSize.Z)); // TOP_5
	OutPositions.Add(WorldCenter + FVector(-VoxelSize.X,0, VoxelSize.Z)); // TOP_6
	OutPositions.Add(WorldCenter + FVector(-VoxelSize.X, VoxelSize.Y, VoxelSize.Z)); // TOP_7
	
	OutPositions.Add(WorldCenter + FVector(VoxelSize.X,-VoxelSize.Y, 0)); // MIDDLE_9
	OutPositions.Add(WorldCenter + FVector(VoxelSize.X,0, 0)); // MIDDLE_10
	OutPositions.Add(WorldCenter + FVector(VoxelSize.X,VoxelSize.Y, 0)); // MIDDLE_11
	OutPositions.Add(WorldCenter + FVector(0,-VoxelSize.Y, 0)); // MIDDLE_12
	OutPositions.Add(WorldCenter + FVector(0,VoxelSize.Y, 0)); // MIDDLE_13
	OutPositions.Add(WorldCenter + FVector(-VoxelSize.X,-VoxelSize.Y, 0)); // MIDDLE_14
	OutPositions.Add(WorldCenter + FVector(-VoxelSize.X,0, 0)); // MIDDLE_15
	OutPositions.Add(WorldCenter + FVector(-VoxelSize.X,VoxelSize.Y, 0)); // MIDDLE_16
	
	OutPositions.Add(WorldCenter + FVector(VoxelSize.X,-VoxelSize.Y, -VoxelSize.Z)); // BOTTOM_17
	OutPositions.Add(WorldCenter + FVector(VoxelSize.X,0, -VoxelSize.Z)); // BOTTOM_18
	OutPositions.Add(WorldCenter + FVector(VoxelSize.X,VoxelSize.Y, -VoxelSize.Z)); // BOTTOM_19
	OutPositions.Add(WorldCenter + FVector(0, -VoxelSize.Y, -VoxelSize.Z)); // BOTTOM_20
	OutPositions.Add(WorldCenter + FVector(0,0, -VoxelSize.Z)); // BOTTOM_25
	OutPositions.Add(WorldCenter + FVector(0,VoxelSize.Y, -VoxelSize.Z)); // BOTTOM_21
	OutPositions.Add(WorldCenter + FVector(-VoxelSize.X,-VoxelSize.Y, -VoxelSize.Z)); // BOTTOM_22
	OutPositions.Add(WorldCenter + FVector(-VoxelSize.X,0, -VoxelSize.Z)); // BOTTOM_23
	OutPositions.Add(WorldCenter + FVector(-VoxelSize.X,VoxelSize.Y, -VoxelSize.Z)); // BOTTOM_24
}

void FNProcGenUtils::GetVoxelQueryLevelBoundsEndPoints(const FVector& WorldCenter, const FBox& LevelBounds, TArray<FVector>& OutPositions )
{
	FVector IntersectionPoint;
	const FBox ExpendedBounds = LevelBounds.ExpandBy(10.f);
	GetVoxelQueryPoints(WorldCenter,FVector::OneVector, OutPositions); // We do not care about the size for this
	const int Count = OutPositions.Num();
	for (int i = 0; i < Count; i++)
	{
		FVector Direction = OutPositions[i] - WorldCenter;
		Direction.Normalize();
		if (RayAABBIntersection(WorldCenter, Direction, ExpendedBounds, IntersectionPoint))
		{
			OutPositions[i] = IntersectionPoint;
		}
		else
		{
			// No hit - just return center
			OutPositions[i] = WorldCenter;
		}
	}
	
}
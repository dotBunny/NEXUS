// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenUtils.h"

#include "KismetTraceUtils.h"
#include "NArrayUtils.h"
#include "NProcGenSettings.h"
#include "Organ/NOrganVolume.h"
#include "Chaos/Convex.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Macros/NFlagsMacros.h"
#include "Organ/NOrganComponent.h"
#include "Runtime/Experimental/Chaos/Private/Chaos/PhysicsObjectInternal.h"

#define LOCTEXT_NAMESPACE "NexusProcGen"

FBox FNProcGenUtils::CalculatePlayableBounds(ULevel* InLevel, const FNCellBoundsGenerationSettings& Settings)
{
	FBox LevelBounds(ForceInit);
	if (InLevel)
	{
		const int32 NumActors = InLevel->Actors.Num();
		FScopedSlowTask Task = FScopedSlowTask(NumActors, LOCTEXT("NProcGen_FNProcGenUtils_CalculatePlayableBounds", "Calculate Playable Bounds"));
		Task.MakeDialog(false);
	
		for (int32 ActorIndex = 0; ActorIndex < NumActors; ++ActorIndex)
		{
			
			const AActor* Actor = InLevel->Actors[ActorIndex];
			Task.EnterProgressFrame(1);
	
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
		
		FScopedSlowTask ActorTask = FScopedSlowTask(NumActors, LOCTEXT("NProcGen_FNProcGenUtils_CalculateConvexHull_Actor", "Calculate Convex Hull - Actors"));
		ActorTask.MakeDialog(false);
		
		Vertices.Reserve(NumActors * 8);
		for (int32 ActorIndex = 0; ActorIndex < InLevel->Actors.Num() ; ++ActorIndex)
		{
			ActorTask.EnterProgressFrame(1);
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

	FScopedSlowTask ChaosTask = FScopedSlowTask(2, LOCTEXT("NProcGen_FNProcGenUtils_CalculateConvexHull_Chaos", "Calculate Convex Hull - Chaos"));
	ChaosTask.MakeDialog(false);
	ChaosTask.EnterProgressFrame(1);
	Chaos::FConvexBuilder::FConvexBuilder::Build(Vertices, OutPlanes, OutFaceIndices, OutVertices, OutLocalBounds, Settings.GetChaosBuildMethod());
	ChaosTask.EnterProgressFrame(1);
	
	
	
	// Construct FVector Vertices
	const int VerticesCount = OutVertices.Num();
	const int IndicesCount = OutFaceIndices.Num();
	
	FScopedSlowTask BuildTask = FScopedSlowTask(VerticesCount + IndicesCount, LOCTEXT("NProcGen_FNProcGenUtils_CalculateConvexHull_Build", "Calculate Convex Hull - Build Mesh"));
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
	//Mesh.ConvertToTriangles();
	//Mesh.Validate();
	Mesh.bIsChaosGenerated = true;
	Mesh.bIsConvex = true;
	Mesh.bHasNonTris = Mesh.CheckNonTris();
	
	return Mesh;
}

FNCellVoxelData FNProcGenUtils::CalculateVoxelData(ULevel* InLevel, const FNCellVoxelGenerationSettings& Settings)
{
	
	// TODO: We probably could use the voxel data to actually generate the overall bounds to avoid the double parse of the actors in the level
	FNCellVoxelData ReturnData;
	
	if (InLevel)
	{
		// Settings
		const UWorld* World = InLevel->GetWorld();
		const FVector UnitSize = UNProcGenSettings::Get()->UnitSize;
		const ECollisionChannel CollisionChannel = Settings.CollisionChannel;
		const FVector HalfUnitSize = UnitSize * 0.5f;
		
		const FVector FudgeHalfUnitSize = UnitSize * 0.5f;
		
		// STEP 1 - Specific Bounds
		FBox Bounds(ForceInit);
		const int32 NumActors = InLevel->Actors.Num();
		FScopedSlowTask BoundsTask = FScopedSlowTask(NumActors, LOCTEXT("NProcGen_FNProcGenUtils_CalculateVoxelData_Bounds", "Build Voxel World"));
		BoundsTask.MakeDialog(false);
		for (int32 ActorIndex = 0; ActorIndex < NumActors; ++ActorIndex)
		{
			const AActor* Actor = InLevel->Actors[ActorIndex];
			BoundsTask.EnterProgressFrame(1);

			if (Actor && Actor->IsLevelBoundsRelevant())
			{
				// Ignore Tags
				if (FNArrayUtils::ContainsAny(Actor->Tags, Settings.ActorIgnoreTags)) continue;
				const FBox ActorBox = Actor->GetComponentsBoundingBox(Settings.bIncludeNonColliding);
				if (ActorBox.IsValid)
				{
					Bounds+= ActorBox;
				}
			}
		}
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
		
		FCollisionShape BoxShape = FCollisionShape::MakeBox(FudgeHalfUnitSize);
		FCollisionQueryParams Params = FCollisionQueryParams(TEXT("CalculateVoxelData"), true);
		FCollisionObjectQueryParams ObjectParams = FCollisionObjectQueryParams(CollisionChannel);
	
		
		// STEP 2 - Broad Trace
		FScopedSlowTask BroadTraceTask = FScopedSlowTask(Count, LOCTEXT("NProcGen_FNProcGenUtils_CalculateVoxelData_BroadTrace", "Broad Trace"));
		BroadTraceTask.MakeDialog(false);
		
		// DEBUG
		FlushPersistentDebugLines(World);
		
		// Origin
		//DrawDebugPoint(World, ReturnData.Origin, 10.f, FColor::Green, true, 0.f, 2.0f);
		
		// Min/Max
		// DrawDebugPoint(World, ReturnData.Origin + ((FVector(0, 0, 0) * UnitSize) + HalfUnitSize), 10.f, FColor::Yellow, true, 0.f, 2.0f);
		// DrawDebugPoint(World, ReturnData.Origin + ((FVector(SizeX-1, SizeY-1, SizeZ-1) * UnitSize) + HalfUnitSize), 10.f, FColor::Yellow, true, 0.f, 2.0f);
		//
		// // Axis
		// for (int x = 0; x < SizeX; x++)
		// {
		// 	DrawDebugPoint(World, ReturnData.Origin + ((FVector(x, 0, 0) * UnitSize) + HalfUnitSize), 10.f, FColor::Red, true, 0.f, 2.0f);
		// }
		// for (int x = 0; x < SizeY; x++)
		// {
		// 	DrawDebugPoint(World, ReturnData.Origin + ((FVector(0, x, 0) * UnitSize) + HalfUnitSize), 10.f, FColor::Green, true, 0.f, 2.0f);
		// }
		// for (int x = 0; x < SizeZ; x++)
		// {
		// 	DrawDebugPoint(World, ReturnData.Origin + ((FVector(0, 0, x) * UnitSize) + HalfUnitSize), 10.f, FColor::Blue, true, 0.f, 2.0f);
		// }
		
		// We iterate over the array by axis to minimize inverse calculations

		TArray<FVector> RayEndPoints;
		TArray<FHitResult> ObjectHits;
		
		for (int x = 0; x < SizeX; x++)
		{
			for (int y = 0; y < SizeY; y++)
			{
				for (int z = 0; z < SizeZ; z++)
				{
					BroadTraceTask.EnterProgressFrame(1);
					FVector VoxelCenter = ReturnData.Origin + ((FVector(x, y, z) * UnitSize) + HalfUnitSize);
					
					// Create Rays
					RayEndPoints.Empty();
					GetVoxelTestRayEndPoints(VoxelCenter, Bounds, RayEndPoints);
			
					bool bIsInside = false;
					for (int j = 0; j < 26; j++)
					{
						bool bHitObjects =  World->LineTraceMultiByObjectType(ObjectHits, RayEndPoints[j], VoxelCenter, ObjectParams, Params);
						//DrawDebugLineTraceMulti(World, RayEndPoints[j], VoxelCenter, EDrawDebugTrace::Persistent, bFollowupHit, RayHits, FLinearColor::Yellow, FLinearColor::Red, -1.f);
						if (bHitObjects)
						{
							
							// We've hit objects from the outside of the bounds inward, which now means we need to 
							// determine if we are inside one of them.
							
							const int Hits = ObjectHits.Num();
							if (Hits == 0) continue; // No hits, void
							
							for (int  k = 0; k < Hits; k++)
							{
								
								
								// We need to now see if we are just passing through or not
								if (ObjectHits[k].PenetrationDepth > 0)
								{
									//UE_LOG(LogTemp, Warning, TEXT("PENETRACTION Voxel %d,%d,%d - Ray %d hit %s"), x,y,z, j,  *ObjectHits[k].GetActor()->GetActorLabel());
								}
								
								
								//UE_LOG(LogTemp, Warning, TEXT("Voxel %d,%d,%d - Ray %d hit %s"), x,y,z, j,  *ObjectHits[k].GetActor()->GetActorLabel());
							}
							
							
							if (FMath::Modulo(ObjectHits.Num(), 2) != 0)
							{
								bIsInside = true;
							}
						}
					}
			
					if (bIsInside)
					{
						// Outside
						DrawDebugPoint(World, VoxelCenter, 10.f, FColor::Blue, true, 0.f, 2.0f);
					}
				}
			}
		}
	}
	
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

void FNProcGenUtils::GetVoxelTestRayEndPoints(const FVector& WorldCenter, const FBox& LevelBounds, TArray<FVector>& OutPositions )
{
	/** Directional Vectors (26)
	 TOP			MIDDLE				BOTTOM
	 0 - 1 - 2		9 -  10 -  11		17 -  18 -  19
	 |       |		|           |		|           |
	 3   8   4		12    X    13		20    25    21
	 |       |		|		    |		|		    |
	 5 - 6 - 7		14 - 15 -  16		22 -  23 -  24
	 */
	FVector IntersectionPoint;
	FBox ExpendedBounds = LevelBounds.ExpandBy(10.f);
	
	
	// TOP LOOP
	
	FVector Top_0 = (WorldCenter + FVector(1,-1, 1)) - WorldCenter;
	Top_0.Normalize();
	if (RayAABBIntersection(WorldCenter, Top_0, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Top_1 = (WorldCenter + FVector(1,0, 1)) - WorldCenter;
	Top_1.Normalize();
	if (RayAABBIntersection(WorldCenter, Top_1, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Top_2 = (WorldCenter + FVector(1,1, 1)) - WorldCenter;
	Top_2.Normalize();
	if (RayAABBIntersection(WorldCenter, Top_2, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Top_3 = (WorldCenter + FVector(0,-1, 1)) - WorldCenter;
	Top_3.Normalize();
	if (RayAABBIntersection(WorldCenter, Top_3, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Top_8 = (WorldCenter + FVector::UpVector) - WorldCenter;
	Top_8.Normalize();
	if (RayAABBIntersection(WorldCenter, Top_8, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Top_4 =  (WorldCenter + FVector(0,1, 1)) - WorldCenter;
	Top_4.Normalize();
	if (RayAABBIntersection(WorldCenter, Top_4, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Top_5 =  (WorldCenter + FVector(-1,-1, 1)) - WorldCenter;
	Top_5.Normalize();
	if (RayAABBIntersection(WorldCenter, Top_5, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Top_6 = (WorldCenter + FVector(-1,0, 1)) - WorldCenter;
	Top_6.Normalize();
	if (RayAABBIntersection(WorldCenter, Top_6, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Top_7 = (WorldCenter + FVector(-1,1, 1)) - WorldCenter;
	Top_7.Normalize();
	if (RayAABBIntersection(WorldCenter, Top_7, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	
	// MIDDLE
	FVector Middle_9 = (WorldCenter + FVector(1,-1, 0)) - WorldCenter;
	Middle_9.Normalize();
	if (RayAABBIntersection(WorldCenter, Middle_9, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Middle_10 = (WorldCenter + FVector(1,0, 0)) - WorldCenter;
	Middle_10.Normalize();
	if (RayAABBIntersection(WorldCenter, Middle_10, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Middle_11 = (WorldCenter + FVector(1,1, 0)) - WorldCenter;
	Middle_11.Normalize();
	if (RayAABBIntersection(WorldCenter, Middle_11, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Middle_12 = (WorldCenter + FVector::LeftVector) - WorldCenter;
	Middle_12.Normalize();
	if (RayAABBIntersection(WorldCenter, Middle_12, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Middle_13 =  (WorldCenter + FVector::RightVector) - WorldCenter;
	Middle_13.Normalize();
	if (RayAABBIntersection(WorldCenter, Middle_13, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Middle_14 =  (WorldCenter + FVector(-1,-1, 0)) - WorldCenter;
	Middle_14.Normalize();
	if (RayAABBIntersection(WorldCenter, Middle_14, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Middle_15 = (WorldCenter + FVector(-1,0, 0)) - WorldCenter;
	Middle_15.Normalize();
	if (RayAABBIntersection(WorldCenter, Middle_15, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Middle_16 = (WorldCenter + FVector(-1,1, 0)) - WorldCenter;
	Middle_16.Normalize();
	if (RayAABBIntersection(WorldCenter, Middle_16, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	// BOTTOM LOOP
	
	FVector Bottom_17 = (WorldCenter + FVector(1,-1, -1)) - WorldCenter;
	Bottom_17.Normalize();
	if (RayAABBIntersection(WorldCenter, Bottom_17, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Bottom_18 = (WorldCenter + FVector(1,0, -1)) - WorldCenter;
	Bottom_18.Normalize();
	if (RayAABBIntersection(WorldCenter, Bottom_18, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Bottom_19 = (WorldCenter + FVector(1,1, -1)) - WorldCenter;
	Bottom_19.Normalize();
	if (RayAABBIntersection(WorldCenter, Bottom_19, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Bottom_20 = (WorldCenter + FVector(0,-1, -1)) - WorldCenter;
	Bottom_20.Normalize();
	if (RayAABBIntersection(WorldCenter, Bottom_20, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Bottom_25 = (WorldCenter + FVector::DownVector) - WorldCenter;
	Bottom_25.Normalize();
	if (RayAABBIntersection(WorldCenter, Bottom_25, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Bottom_21 =  (WorldCenter + FVector(0,1, -1)) - WorldCenter;
	Bottom_21.Normalize();
	if (RayAABBIntersection(WorldCenter, Bottom_21, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Bottom_22 =  (WorldCenter + FVector(-1,-1, -1)) - WorldCenter;
	Bottom_22.Normalize();
	if (RayAABBIntersection(WorldCenter, Bottom_22, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Bottom_23 = (WorldCenter + FVector(-1,0, -1)) - WorldCenter;
	Bottom_23.Normalize();
	if (RayAABBIntersection(WorldCenter, Bottom_23, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
	
	FVector Bottom_24 = (WorldCenter + FVector(-1,1, -1)) - WorldCenter;
	Bottom_24.Normalize();
	if (RayAABBIntersection(WorldCenter, Bottom_24, ExpendedBounds, IntersectionPoint))
	{
		OutPositions.Add(IntersectionPoint);
	}
}

#undef LOCTEXT_NAMESPACE

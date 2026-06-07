// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyUtils.h"

#include "NArrayUtils.h"
#include "NLevelUtils.h"
#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblySettings.h"
#include "Organ/NOrganVolume.h"
#include "Chaos/Convex.h"
#include "Organ/NOrganComponent.h"
#include "Misc/ScopedSlowTask.h"

FBox FNWorldAssemblyUtils::CalculatePlayableBounds(ULevel* InLevel, const FNCellBoundsGenerationSettings& Settings)
{
	FBox LevelBounds(ForceInit);
	
	// Go home early
	if (InLevel == nullptr)
	{
		return MoveTemp(LevelBounds);
	}
	
	TArray<const AActor*> IgnoredActors;
	
	// Prefill Ignored Actors to effect downstream
	ANCellActor* CellActor = nullptr;
	UNCellRootComponent* CellRoot = FNWorldAssemblyRegistry::GetCellRootComponentFromLevel(InLevel);
	if (CellRoot != nullptr)
	{
		CellActor = CellRoot->GetNCellActor();	
	}
	if (CellActor != nullptr)
	{
		CellActor->AppendAuthorTimeActors(IgnoredActors);
	}
	
	FNLevelUtils::DetermineLevelBounds(InLevel, LevelBounds, IgnoredActors, Settings.ActorIgnoreTags, Settings.bIncludeEditorOnly, Settings.bIncludeNonColliding);
	
	return MoveTemp(LevelBounds);
}


FNRawMesh FNWorldAssemblyUtils::CalculateConvexHull(ULevel* InLevel, const FNCellHullGenerationSettings& Settings)
{
	FNRawMesh Mesh;
	TArray<Chaos::FConvex::FVec3Type> Vertices;
	
	if (InLevel == nullptr)
	{
		return MoveTemp(Mesh);
	}
	
	// Check for Cell Actor
	ANCellActor* CellActor = nullptr;
	UNCellRootComponent* CellRoot = FNWorldAssemblyRegistry::GetCellRootComponentFromLevel(InLevel);
	if (CellRoot != nullptr)
	{
		CellActor = CellRoot->GetNCellActor();	
	}
	
	FVector BoxVertices[8];
	const int32 NumActors = InLevel->Actors.Num();
	
	FScopedSlowTask ActorTask = FScopedSlowTask(NumActors, NSLOCTEXT("NexusWorldAssembly", "Task_CalculateConvexHull_Actor", "Calculate Convex Hull - Actors"));
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
			
			// Don't bother with transient actors
			if (Actor->HasAnyFlags(RF_Transient)) continue;
			
			// Ignore Tags
			if (FNArrayUtils::ContainsAny(Actor->Tags, Settings.ActorIgnoreTags)) continue;
			
			// Author Time Only
			if (CellActor != nullptr && CellActor->IsAuthorTimeActor(Actor)) continue;
			
			if (FNArrayUtils::ContainsAny(Actor->Tags, Settings.ActorIgnoreTags)) continue;
			
			FBox ActorBox = Actor->GetComponentsBoundingBox(Settings.bIncludeNonColliding);
			if (ActorBox.IsValid && 
				(ActorBox.GetExtent().X > 0 && ActorBox.GetExtent().Y > 0 && ActorBox.GetExtent().Z > 0))
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
	
	TArray<Chaos::FConvex::FPlaneType> OutPlanes;
	TArray<TArray<int32>> OutFaceIndices;
	TArray<Chaos::FConvex::FVec3Type> OutVertices;
	Chaos::FConvex::FAABB3Type OutLocalBounds;

	FScopedSlowTask ChaosTask = FScopedSlowTask(2, NSLOCTEXT("NexusWorldAssembly", "Task_CalculateConvexHull_Chaos", "Calculate Convex Hull - Chaos"));
	ChaosTask.MakeDialog(false);
	ChaosTask.EnterProgressFrame(1);
	Chaos::FConvexBuilder::FConvexBuilder::Build(Vertices, OutPlanes, OutFaceIndices, OutVertices, OutLocalBounds, Settings.GetChaosBuildMethod());
	ChaosTask.EnterProgressFrame(1);
	
	// Construct FVector Vertices
	const int32 VerticesCount = OutVertices.Num();
	const int32 IndicesCount = OutFaceIndices.Num();
	
	FScopedSlowTask BuildTask = FScopedSlowTask(VerticesCount + IndicesCount, NSLOCTEXT("NexusWorldAssembly", "Task_CalculateConvexHull_Build", "Calculate Convex Hull - Build Mesh"));
	BuildTask.MakeDialog(false);
	
	Mesh.Vertices.Reserve(VerticesCount);
	FVector CenterCalc;
	FBox BoundingBox(ForceInit);
	for (int32 i = 0; i < VerticesCount; i++)
	{
		BuildTask.EnterProgressFrame(1);
		Mesh.Vertices.Add(FVector(OutVertices[i][0], OutVertices[i][1], OutVertices[i][2]));
		
		FVector& CreatedPoint = Mesh.Vertices.Last();
		CenterCalc += CreatedPoint;
		BoundingBox += CreatedPoint;
	}

	// Capture the polygonal faces straight from Chaos as the canonical face description, then start Loops
	// as a copy so triangulation below has something to work on. FaceLoops remains polygonal and is what
	// CheckConvex consults — that avoids the per-triangle coplanar-drift false-negative after a vertex edit.
	Mesh.FaceLoops.Reserve(IndicesCount);
	for (int32 i = 0; i < IndicesCount; i++)
	{
		BuildTask.EnterProgressFrame(1);
		Mesh.FaceLoops.Add(FNRawMeshLoop(OutFaceIndices[i]));
	}
	Mesh.Loops = Mesh.FaceLoops;

	Mesh.Center = (VerticesCount > 0) ? (CenterCalc / VerticesCount) : FVector::ZeroVector;
	Mesh.Bounds = BoundingBox;

	// Because the generator will have processed the mesh and even though it could have ngons it is still a convex hull
	Mesh.bIsChaosGenerated = true;
	Mesh.bIsConvex = true;
	Mesh.bHasBounds = true;

	// ConvertToTriangles self-early-outs when Loops is already all-tris, so call it unconditionally and
	// skip the extra CheckNonTris pass that used to gate it.
	Mesh.ConvertToTriangles();
	Mesh.bHasNonTris = false;

	return Mesh;
}

FNCellVoxelData FNWorldAssemblyUtils::CalculateVoxelData(ULevel* InLevel, const FNCellVoxelGenerationSettings& Settings)
{
	// TODO: We probably could use the voxel data to actually generate the overall bounds to avoid the double parse of the actors in the level
	FNCellVoxelData ReturnData;
	
	// Go home early
	if (InLevel == nullptr)
	{
		return MoveTemp(ReturnData);
	}
	
	// Prefill Ignored Actors to effect downstream
	TArray<const AActor*> IgnoredActors;
	ANCellActor* CellActor = nullptr;
	UNCellRootComponent* CellRoot = FNWorldAssemblyRegistry::GetCellRootComponentFromLevel(InLevel);
	if (CellRoot != nullptr)
	{
		CellActor = CellRoot->GetNCellActor();	
	}
	if (CellActor != nullptr)
	{
		CellActor->AppendAuthorTimeActors(IgnoredActors);
	}
	
	// Settings
	const UWorld* World = InLevel->GetWorld();
	const FVector UnitSize = UNWorldAssemblySettings::Get()->VoxelSize;
	const ECollisionChannel CollisionChannel = Settings.CollisionChannel;
	const FVector HalfUnitSize = UnitSize * 0.5f;
	
	// STEP 1 - Specific Bounds / Ignore Actors
	FBox Bounds(ForceInit);
	FNLevelUtils::DetermineLevelBounds(InLevel, Bounds, IgnoredActors, Settings.ActorIgnoreTags, Settings.bIncludeEditorOnly, Settings.bIncludeNonColliding);
	
	ReturnData.Origin = Bounds.Min;
	
	const FBox UnitBounds = FBox(
				FNVectorUtils::GetFurthestGridIntersection(Bounds.Min, UnitSize),
				FNVectorUtils::GetFurthestGridIntersection(Bounds.Max, UnitSize));
	
	const FVector BoundsSize = UnitBounds.GetSize();
	const uint32 SizeX = FMath::RoundToInt(BoundsSize.X);
	const uint32 SizeY = FMath::RoundToInt(BoundsSize.Y);
	const uint32 SizeZ = FMath::RoundToInt(BoundsSize.Z);
	
	// Setup array
	ReturnData.Resize(SizeX, SizeY, SizeZ);
	const size_t Count = ReturnData.GetCount();
	
	FCollisionQueryParams Params = FCollisionQueryParams(TEXT("CalculateVoxelData"), true);
	Params.AddIgnoredActors(IgnoredActors);
	
	// STEP 2 - Broad Trace
	FScopedSlowTask BroadTraceTask = FScopedSlowTask(Count, NSLOCTEXT("NexusWorldAssembly", "Task_CalculateVoxelData_BroadTrace", "Broad Trace"));
	BroadTraceTask.MakeDialog(false);

	// We iterate over the array by axis to minimize inverse calculations
	TArray<FVector> RayEndPoints;
	FHitResult SingleHit;
	TArray<FHitResult> ObjectHits;
	
	// Our initial box shape is slightly larger than the actual voxel unit size as to always detect collisions right on the extents.
	FCollisionShape BoxShape = FCollisionShape::MakeBox(HalfUnitSize + FVector(0.001f, 0.001f, 0.001f));
	TArray<uint32> SurroundingIndices;
	
	// #SONARQUBE-DISABLE Need to loop depth to handle dimensions
	for (uint32 x = 0; x < SizeX; x++)
	{
		for (uint32 y = 0; y < SizeY; y++)
		{
			for (uint32 z = 0; z < SizeZ; z++)
			{
				const size_t VoxelIndex = ReturnData.GetIndex(x,y,z);
				BroadTraceTask.EnterProgressFrame(1);
				FVector VoxelCenter = ReturnData.Origin + ((FVector(x, y, z) * UnitSize) + HalfUnitSize);
				
				// Standard Overlap Check
				bool const bHit = World ? World->SweepSingleByChannel(SingleHit, VoxelCenter, VoxelCenter, FQuat::Identity, CollisionChannel, BoxShape, Params) : false;
				if (bHit)
				{
					ReturnData.AddFlag(VoxelIndex, ENCellVoxel::Occupied);
				}
			}
		}
	}
	// #SONARQUBE-ENABLE
	
	return MoveTemp(ReturnData);
}

int32 FNWorldAssemblyUtils::GetCellActorCountFromLevel(const ULevel* Level)
{
	int32 Count = 0;
	for (auto ActorIt = Level->Actors.CreateConstIterator(); ActorIt; ++ActorIt )
	{
		if (ActorIt->IsA<ANCellActor>())
		{
			Count++;
		}
	}
	return Count;
}

int32 FNWorldAssemblyUtils::GetCellActorCountFromWorld(const UWorld* World, const bool bIgnoreInstancedLevels)
{
	int32 Count = 0;
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

ANCellActor* FNWorldAssemblyUtils::GetCellActorFromLevel(const ULevel* Level)
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

ANCellActor* FNWorldAssemblyUtils::GetCellActorFromWorld(const UWorld* World, const bool bIgnoreInstancedLevels)
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

TArray<UNOrganComponent*> FNWorldAssemblyUtils::GetOrganComponentsFromLevel(const ULevel* InLevel, const bool bSorted)
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
	
	if (bSorted)
	{
		Result.Sort([](const UNOrganComponent& A, const UNOrganComponent& B) {
			return A.Identifier < B.Identifier;
		});
	}
    
	return MoveTemp(Result);

}

TArray<ANOrganVolume*> FNWorldAssemblyUtils::GetOrganVolumesFromLevel(const ULevel* InLevel)
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

TArray<ANOrganVolume*> FNWorldAssemblyUtils::GetOrganVolumesFromWorld(const UWorld* World, bool bIgnoreInstancedLevels)
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

TArray<FVector2D> FNWorldAssemblyUtils::GetSocketPoints2D(const FIntVector2& Units, const FVector2D& UnitSize)
{
	TArray<FVector2D> Points;
	Points.Reserve( Units.X * Units.Y);
	const double UsableWidth = ((Units.X * UnitSize.X) - UnitSize.X);
	const double UsableHeight = ((Units.Y * UnitSize.Y) - UnitSize.Y);
	const double WidthStart = -(UsableWidth * 0.5f);
	const double HeightStart = -(UsableHeight * 0.5f);

	for (int32 i = 0; i < Units.X; i++)
	{
		for ( int32 y = 0; y < Units.Y; y++)
		{
			Points.Add(FVector2D(WidthStart + (i * UnitSize.X), HeightStart + (y * UnitSize.Y)));
		}
	}
	return MoveTemp(Points);
}

TArray<FVector> FNWorldAssemblyUtils::GetCenteredWorldCornerPoints2D(
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

	return MoveTemp(ReturnPositions);
}

void FNWorldAssemblyUtils::GetVoxelQueryPoints(const FVector& WorldCenter, const FVector& VoxelSize, TArray<FVector>& OutPositions)
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

void FNWorldAssemblyUtils::GetVoxelQueryLevelBoundsEndPoints(const FVector& WorldCenter, const FBox& LevelBounds, TArray<FVector>& OutPositions )
{
	FVector IntersectionPoint;
	const FBox ExpendedBounds = LevelBounds.ExpandBy(10.f);
	GetVoxelQueryPoints(WorldCenter,FVector::OneVector, OutPositions); // We do not care about the size for this
	const int32 Count = OutPositions.Num();
	for (int32 i = 0; i < Count; i++)
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
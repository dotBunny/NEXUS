// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenUtils.h"
#include "NArrayUtils.h"
#include "NProcGenSettings.h"
#include "Organ/NOrganVolume.h"
#include "Chaos/Convex.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Organ/NOrganComponent.h"

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
	
	FNCellVoxelData ReturnData;
	
	if (InLevel)
	{
		// TODO: Pass in existing?
		FNCellBoundsGenerationSettings BoundsSettings;
		BoundsSettings.ActorIgnoreTags = Settings.ActorIgnoreTags;
		BoundsSettings.bIncludeNonColliding = Settings.bIncludeNonColliding;
		BoundsSettings.bIncludeEditorOnly = Settings.bIncludeEditorOnly;
		
		const FBox Bounds = CalculatePlayableBounds(InLevel, BoundsSettings);
		const FVector UnitSize = UNProcGenSettings::Get()->UnitSize;
		const FVector HalfUnitSize = UnitSize * 0.5f;
		const FVector FudgeHalfUnitSize = UnitSize * 0.5f;
		
		ReturnData.Origin = Bounds.Min;
		
		const FBox UnitBounds = FBox(
					FNVectorUtils::GetFurthestGridIntersection(Bounds.Min, UnitSize),
					FNVectorUtils::GetFurthestGridIntersection(Bounds.Max, UnitSize));
		
		const size_t SizeX = FMath::FloorToInt(FMath::Abs(UnitBounds.Min.X) + FMath::Abs(UnitBounds.Max.X));	
		const size_t SizeY = FMath::FloorToInt(FMath::Abs(UnitBounds.Min.Y) + FMath::Abs(UnitBounds.Max.Y));	
		const size_t SizeZ = FMath::FloorToInt(FMath::Abs(UnitBounds.Min.Y) + FMath::Abs(UnitBounds.Max.Z));	
		
		ReturnData.Resize(SizeX, SizeY, SizeZ);
		
		const UWorld* World = InLevel->GetWorld();
		const size_t Count = ReturnData.GetCount();
		FScopedSlowTask TraceTask = FScopedSlowTask(Count, LOCTEXT("NProcGen_FNProcGenUtils_CalculateVoxelData", "Calculate Voxel Data"));
		TraceTask.MakeDialog(false);
		
		FlushPersistentDebugLines(World);
		TArray<FHitResult> OutHits;
		FCollisionShape BoxShape = FCollisionShape::MakeBox(FudgeHalfUnitSize);
		FCollisionQueryParams Params = FCollisionQueryParams(TEXT("CalculateVoxelData"), true);
	
		// We iterate over the array by axis to minimize inverse calculations
		for (int x = 0; x < SizeX; x++)
		{
			for (int y = 0; y < SizeY; y++)
			{
				for (int z = 0; z < SizeZ; z++)
				{
					TraceTask.EnterProgressFrame(1);
					DrawDebugPoint(World, ReturnData.Origin, 5.f, FColor::Yellow, true);
					FVector VoxelCenter = ReturnData.Origin + ((FVector(x, y, z) * UnitSize) + HalfUnitSize);
					
					//static const FName BoxTraceMultiName(TEXT("BoxTraceMulti"));
					//FCollisionQueryParams Params = ConfigureCollisionParams(BoxTraceMultiName, bTraceComplex, ActorsToIgnore, bIgnoreSelf, WorldContextObject);
					const ECollisionChannel CollisionChannel = ECollisionChannel::ECC_WorldStatic;
					bool const bHit = World ? World->SweepMultiByChannel(OutHits, VoxelCenter, VoxelCenter, FQuat::Identity, CollisionChannel, BoxShape, Params) : false;
					
					if (bHit)
					{
						DrawDebugBox(World, VoxelCenter, HalfUnitSize, FColor::Red, true, 0.5f, 0, 2.0f);
					}
					else
					{
						//DrawDebugBox(World, VoxelCenter, HalfUnitSize, FColor::Red, true, 0.5f, 0, 2.0f);
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

#undef LOCTEXT_NAMESPACE

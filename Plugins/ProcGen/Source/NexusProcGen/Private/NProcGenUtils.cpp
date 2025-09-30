// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenUtils.h"
#include "NProcGenComponent.h"
#include "NArrayUtils.h"
#include "NProcGenSettings.h"
#include "NProcGenVolume.h"
#include "Chaos/Convex.h"


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

int FNProcGenUtils::GetNCellActorCountFromLevel(const ULevel* Level)
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

int FNProcGenUtils::GetNCellActorCountFromWorld(const UWorld* World, const bool bIgnoreInstancedLevels)
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

ANCellActor* FNProcGenUtils::GetNCellActorFromLevel(const ULevel* Level)
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

ANCellActor* FNProcGenUtils::GetNCellActorFromWorld(const UWorld* World, const bool bIgnoreInstancedLevels)
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

TArray<UNProcGenComponent*> FNProcGenUtils::GetNProcGenComponentsFromLevel(const ULevel* InLevel)
{
	TArray<UNProcGenComponent*> Result;
	if (InLevel == nullptr) return Result;
    
	for (auto ActorIt = InLevel->Actors.CreateConstIterator(); ActorIt; ++ActorIt)
	{
		if (const AActor* Actor = ActorIt->Get())
		{
			TArray<UNProcGenComponent*> Components;
			Actor->GetComponents<UNProcGenComponent>(Components);
			Result.Append(Components);
		}
	}
    
	return Result;

}

TArray<UNProcGenComponent*> FNProcGenUtils::GetNProcGenComponentsFromWorld(const UWorld* World, bool bIgnoreInstancedLevels)
{
	TArray<UNProcGenComponent*> Result;
	if (World == nullptr) return Result;
    
	for (const ULevel* Level : World->GetLevels())
	{
		if (bIgnoreInstancedLevels && Level->IsInstancedLevel()) continue;
        
		TArray<UNProcGenComponent*> LevelComponents = GetNProcGenComponentsFromLevel(Level);
		Result.Append(LevelComponents);
	}
    
	return Result;

}

TArray<ANProcGenVolume*> FNProcGenUtils::GetNProcGenVolumesFromLevel(const ULevel* InLevel)
{
	TArray<ANProcGenVolume*> Result;
	if (InLevel == nullptr) return Result;
	for (auto ActorIt = InLevel->Actors.CreateConstIterator(); ActorIt; ++ActorIt )
	{
		if (ActorIt->IsA<ANProcGenVolume>())
		{
			Result.Add(Cast<ANProcGenVolume>(ActorIt->Get()));
		}
	}
	return Result;
}

TArray<ANProcGenVolume*> FNProcGenUtils::GetNProcGenVolumesFromWorld(const UWorld* World, bool bIgnoreInstancedLevels)
{
	TArray<ANProcGenVolume*> Result;
	if (World == nullptr ) return Result;
	
	for (const ULevel* Level : World->GetLevels())
	{
		if (bIgnoreInstancedLevels && Level->IsInstancedLevel()) continue;
		for (auto ActorIt = Level->Actors.CreateConstIterator(); ActorIt; ++ActorIt )
		{
			if (ActorIt->IsA<ANProcGenVolume>())
			{
				Result.Add(Cast<ANProcGenVolume>(ActorIt->Get()));
			}
		}
	}
	return Result;
}




TArray<FVector2D> FNProcGenUtils::GetWorldUnitPoints2D(const ENCellJunctionSize2D& Size, const FVector& UnitSize)
{
	static const float HalfX = UnitSize.X * 0.5f;
	static const float HalfY = UnitSize.Y * 0.5f;
	
	TArray<FVector2D> Points;

	// This is based on a center point expanding out, not a grid starting point.
	switch (Size)
	{
	case ENCellJunctionSize2D::NCJS_1x1:
		Points.Reserve(1);
		Points.Add(FVector2D(0, 0));
		break;
	case ENCellJunctionSize2D::NCJS_1x2: 
		Points.Reserve(2);
		Points.Add(FVector2D(0, HalfY));
		Points.Add(FVector2D(0, -HalfY));
		break;
	case ENCellJunctionSize2D::NCJS_1x3: 
		Points.Reserve(3);
		Points.Add(FVector2D(0, UnitSize.Y));
		Points.Add(FVector2D(0, 0));
		Points.Add(FVector2D(0, -UnitSize.Y));
		break;
	case ENCellJunctionSize2D::NCJS_1x4:
		Points.Reserve(4);
		Points.Add(FVector2D(0, UnitSize.Y + HalfY));
		Points.Add(FVector2D(0,HalfY));
		Points.Add(FVector2D(0,-HalfY));
		Points.Add(FVector2D(0, -(UnitSize.Y + HalfY)));
		break;
		
	case ENCellJunctionSize2D::NCJS_2x1:
		Points.Reserve(2);
		Points.Add(FVector2D(HalfX, 0));
		Points.Add(FVector2D(-HalfX, 0));
		break;

	}

	return MoveTemp(Points);
}

#define N_JUNCTION_SIZE(XUnit,YUnit,ZUnit) \
	static const FVector2D CellJunctionSize_##XUnit##x##YUnit##x##ZUnit = FVector(XUnit*UnitSize.X, YUnit*UnitSize.Y, ZUnit*UnitSize.Z); \
	return CellJunctionSize_##XUnit##x##YUnit##x##ZUnit;

#define N_JUNCTION_SIZE_2D(XUnit,YUnit) \
	static const FVector2D CellJunctionSize2D_##XUnit##x##YUnit = FVector2D(XUnit*UnitSize.X, YUnit*UnitSize.Y); \
	return CellJunctionSize2D_##XUnit##x##YUnit;

FVector2D FNProcGenUtils::GetWorldSize2D(const ENCellJunctionSize2D& Size, const FVector& UnitSize)
{
	switch (Size)
	{
	case ENCellJunctionSize2D::NCJS_1x1:
		N_JUNCTION_SIZE_2D(1,1)
	case ENCellJunctionSize2D::NCJS_1x2:
		N_JUNCTION_SIZE_2D(1,2)
	case ENCellJunctionSize2D::NCJS_1x3:
		N_JUNCTION_SIZE_2D(1,3)
	case ENCellJunctionSize2D::NCJS_1x4:
		N_JUNCTION_SIZE_2D(1,4)
	case ENCellJunctionSize2D::NCJS_2x1:
		N_JUNCTION_SIZE_2D(2,1)
	case ENCellJunctionSize2D::NCJS_2x2:
		N_JUNCTION_SIZE_2D(2,2)
	case ENCellJunctionSize2D::NCJS_2x3:
		N_JUNCTION_SIZE_2D(2,3)
	case ENCellJunctionSize2D::NCJS_2x4:
		N_JUNCTION_SIZE_2D(2,4)
	case ENCellJunctionSize2D::NCJS_3x1:
		N_JUNCTION_SIZE_2D(3,1)
	case ENCellJunctionSize2D::NCJS_3x2:
		N_JUNCTION_SIZE_2D(3,2)
	case ENCellJunctionSize2D::NCJS_3x3:
		N_JUNCTION_SIZE_2D(3,3)
	case ENCellJunctionSize2D::NCJS_3x4:
		N_JUNCTION_SIZE_2D(3,4)
	case ENCellJunctionSize2D::NCJS_4x1:
		N_JUNCTION_SIZE_2D(4,1)
	case ENCellJunctionSize2D::NCJS_4x2:
		N_JUNCTION_SIZE_2D(4,2)
	case ENCellJunctionSize2D::NCJS_4x3:
		N_JUNCTION_SIZE_2D(4,3)
	case ENCellJunctionSize2D::NCJS_4x4:
		N_JUNCTION_SIZE_2D(4,4)
	}
	return FVector2D::Zero();
}

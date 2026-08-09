// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyUtils.h"

#include "NActorUtils.h"
#include "NArrayUtils.h"
#include "NLevelUtils.h"
#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblySettings.h"
#include "Organ/NOrganVolume.h"
#include "Chaos/Convex.h"
#include "Components/PrimitiveComponent.h"
#include "Organ/NOrganComponent.h"
#include "Misc/ScopedSlowTask.h"
#include "Types/NRawMeshFactory.h"

namespace NEXUS::WorldAssembly
{
	/**
	 * Spacing used when sampling a landscape whose cell has terrain thinning switched off.
	 *
	 * Landscape has no un-sampled form to fall back on — sampling is how its geometry is obtained at all — so a
	 * zero grid size, which for mesh terrain means "keep every vertex", has no equivalent here and needs a value.
	 */
	inline constexpr double DefaultLandscapeSampleSpacing = 100.0;
}

void FNWorldAssemblyUtils::GridReducePoints(const TArray<FVector>& Points, const FVector& Center, const double GridSize,
	TSet<FIntVector>& SeenCells, TArray<FVector>& OutPoints)
{
	// Terrain topology plays no part here: a cave's inner surface and the far side of a spherical terrain are interior
	// points to a convex hull either way, so they are discarded by the builder whether or not they are thinned first.
	if (GridSize <= 0.0)
	{
		OutPoints.Append(Points);
		return;
	}

	for (const FVector& Point : Points)
	{
		const FIntVector Cell(
			FMath::FloorToInt32(Point.X / GridSize),
			FMath::FloorToInt32(Point.Y / GridSize),
			FMath::FloorToInt32(Point.Z / GridSize));

		bool bAlreadySeen = false;
		SeenCells.Add(Cell, &bAlreadySeen);
		if (bAlreadySeen) continue;

		// Away from the center on each axis independently.
		OutPoints.Add(FVector(
			Point.X >= Center.X ? FMath::CeilToDouble(Point.X / GridSize) * GridSize : FMath::FloorToDouble(Point.X / GridSize) * GridSize,
			Point.Y >= Center.Y ? FMath::CeilToDouble(Point.Y / GridSize) * GridSize : FMath::FloorToDouble(Point.Y / GridSize) * GridSize,
			Point.Z >= Center.Z ? FMath::CeilToDouble(Point.Z / GridSize) * GridSize : FMath::FloorToDouble(Point.Z / GridSize) * GridSize));
	}
}

/**
 * Append a mesh's world-space vertices to a hull point cloud, thinned onto a grid.
 * @param Mesh Source mesh, in its own local space.
 * @param ToWorld Transform placing the mesh in the world.
 * @param GridSize Edge length of a cell, in world units.
 * @param SeenCells Cells already represented; shared across meshes so section seams do not each contribute a copy.
 * @param OutVertices Destination point cloud.
 */
static void AppendGridReducedVertices(const FNRawMesh& Mesh, const FTransform& ToWorld, const double GridSize,
	TSet<FIntVector>& SeenCells, TArray<Chaos::FConvex::FVec3Type>& OutVertices)
{
	TArray<FVector> WorldPoints;
	WorldPoints.Reserve(Mesh.Vertices.Num());
	FBox WorldBounds(ForceInit);
	for (const FVector& Vertex : Mesh.Vertices)
	{
		const FVector World = ToWorld.TransformPosition(Vertex);
		WorldPoints.Add(World);
		WorldBounds += World;
	}
	if (!WorldBounds.IsValid) return;

	TArray<FVector> Reduced;
	FNWorldAssemblyUtils::GridReducePoints(WorldPoints, WorldBounds.GetCenter(), GridSize, SeenCells, Reduced);

	OutVertices.Reserve(OutVertices.Num() + Reduced.Num());
	for (const FVector& Point : Reduced)
	{
		OutVertices.Add(Chaos::FConvex::FVec3Type(Point));
	}
}

FBox FNWorldAssemblyUtils::CalculatePlayableBounds(ULevel* InLevel, const FNCellBoundsGenerationSettings& Settings)
{
	FBox LevelBounds(ForceInit);

	// Go home early
	if (InLevel == nullptr)
	{
		return LevelBounds;
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

	FNLevelBoundsFilter Filter;
	Filter.ActorIgnoreTags = Settings.ActorIgnoreTags;
	Filter.bIncludeEditorOnly = Settings.bIncludeEditorOnly;
	Filter.bIncludeNonColliding = Settings.bIncludeNonColliding;
	Filter.bIncludeTerrain = Settings.bIncludeTerrain;

	FNLevelUtils::DetermineLevelBounds(InLevel, LevelBounds, IgnoredActors, Filter);

	return LevelBounds;
}


FNRawMesh FNWorldAssemblyUtils::CalculateConvexHull(ULevel* InLevel, const FNCellHullGenerationSettings& Settings)
{
	FNRawMesh Mesh;
	TArray<Chaos::FConvex::FVec3Type> Vertices;

	if (InLevel == nullptr)
	{
		return Mesh;
	}

	// Check for Cell Actor
	ANCellActor* CellActor = nullptr;
	UNCellRootComponent* CellRoot = FNWorldAssemblyRegistry::GetCellRootComponentFromLevel(InLevel);
	if (CellRoot != nullptr)
	{
		CellActor = CellRoot->GetNCellActor();
	}

	const int32 NumActors = InLevel->Actors.Num();

	// STEP 1 - Filter the level's actors down to those that should contribute to the hull.
	FScopedSlowTask ActorTask = FScopedSlowTask(NumActors, NSLOCTEXT("NexusWorldAssembly", "Task_CalculateConvexHull_Actor", "Calculate Convex Hull - Actors"));
	ActorTask.MakeDialog(false);

	// Terrain is kept apart from here on: it is the only contributor whose geometry needs thinning before the hull
	// builder sees it, and separating the two collections is what lets the thinning apply to just that half.
	TArray<AActor*> HullActors;
	TArray<AActor*> TerrainActors;
	TArray<AActor*> LandscapeActors;
	HullActors.Reserve(NumActors);
	int32 TerrainActorCount = 0;
	for (int32 ActorIndex = 0; ActorIndex < NumActors; ++ActorIndex)
	{
		ActorTask.EnterProgressFrame(1);
		AActor* Actor = InLevel->Actors[ActorIndex];
		if (Actor == nullptr || !Actor->IsLevelBoundsRelevant()) continue;

		// Check Editor Only
		if (Actor->IsEditorOnly() && !Settings.bIncludeEditorOnly) continue;

		// Terrain authoring apparatus is never geometry, at any setting — see FNActorUtils::IsTerrainAuthoringActor.
		if (FNActorUtils::IsTerrainAuthoringActor(Actor)) continue;

		// Terrain answers to its own setting rather than to the filters below — see FNCellHullGenerationSettings.
		const bool bIsTerrain = FNActorUtils::IsTerrainActor(Actor);
		if (bIsTerrain && !Settings.bIncludeTerrain) continue;

		// Don't bother with transient actors, terrain excepted: Mesh Partition represents an authored terrain in the
		// editor as transient actors spawned into the persistent level, so the blanket skip would leave the hull
		// without the floor the cell stands on.
		if (Actor->HasAnyFlags(RF_Transient) && !bIsTerrain) continue;

		// Ignore Tags
		if (FNArrayUtils::ContainsAny(Actor->Tags, Settings.ActorIgnoreTags)) continue;

		// Author Time Only
		if (CellActor != nullptr && CellActor->IsAuthorTimeActor(Actor)) continue;

		if (bIsTerrain)
		{
			TerrainActorCount++;

			// Landscape is separated again because it is the one terrain with no geometry to extract — it has to be
			// sampled off the physics scene instead. See FNWorldAssemblyUtils::SampleLandscapeSurface.
			if (FNActorUtils::IsLandscapeActor(Actor))
			{
				LandscapeActors.Add(Actor);
			}
			else
			{
				TerrainActors.Add(Actor);
			}
			continue;
		}
		HullActors.Add(Actor);
	}

	// STEP 2 - Pull each qualifying actor's collision geometry (convex/box/sphere/capsule, or the
	// complex-as-simple tri-mesh) and flatten every vertex into world space for the hull builder.
	TArray<FNRawMesh> CollisionMeshes;
	TArray<FTransform> CollisionTransforms;
	FNRawMeshFactory::FromActorsInBounds(HullActors, {}, CollisionMeshes, CollisionTransforms);

	TArray<FNRawMesh> TerrainMeshes;
	TArray<FTransform> TerrainTransforms;
	FNRawMeshFactory::FromActorsInBounds(TerrainActors, {}, TerrainMeshes, TerrainTransforms);

	int32 CollisionVertexCount = 0;
	int32 CollisionTriangleCount = 0;
	for (const FNRawMesh& CollisionMesh : CollisionMeshes)
	{
		CollisionVertexCount += CollisionMesh.Vertices.Num();
		CollisionTriangleCount += CollisionMesh.Loops.Num();
	}

	int32 TerrainVertexCount = 0;
	for (const FNRawMesh& TerrainMesh : TerrainMeshes)
	{
		TerrainVertexCount += TerrainMesh.Vertices.Num();
	}

	Vertices.Reserve(CollisionVertexCount + (Settings.bIncludeNonColliding ? HullActors.Num() * 8 : 0));
	for (int32 MeshIndex = 0; MeshIndex < CollisionMeshes.Num(); ++MeshIndex)
	{
		const FTransform& ToWorld = CollisionTransforms[MeshIndex];
		for (const FVector& Vertex : CollisionMeshes[MeshIndex].Vertices)
		{
			Vertices.Add(Chaos::FConvex::FVec3Type(ToWorld.TransformPosition(Vertex)));
		}
	}

	// STEP 2A - Fold the terrain in, thinned onto a grid unless the setting turns that off. Authored geometry above
	// is taken whole; only terrain arrives dense enough for the reduction to be worth its own pass.
	const int32 VerticesBeforeTerrain = Vertices.Num();
	TSet<FIntVector> SeenCells;
	for (int32 MeshIndex = 0; MeshIndex < TerrainMeshes.Num(); ++MeshIndex)
	{
		const FTransform& ToWorld = TerrainTransforms[MeshIndex];
		if (Settings.TerrainSimplificationGridSize > 0.f)
		{
			AppendGridReducedVertices(TerrainMeshes[MeshIndex], ToWorld, Settings.TerrainSimplificationGridSize, SeenCells, Vertices);
		}
		else
		{
			for (const FVector& Vertex : TerrainMeshes[MeshIndex].Vertices)
			{
				Vertices.Add(Chaos::FConvex::FVec3Type(ToWorld.TransformPosition(Vertex)));
			}
		}
	}

	const int32 VerticesAfterTerrain = Vertices.Num();

	// STEP 2B - Landscape, which has no geometry to extract and is sampled off the physics scene instead. The samples
	// are already grid spaced, so they need no further thinning.
	const double LandscapeSampleSpacing = Settings.TerrainSimplificationGridSize > 0.f
		? Settings.TerrainSimplificationGridSize
		: NEXUS::WorldAssembly::DefaultLandscapeSampleSpacing;

	int32 LandscapeVertexCount = 0;
	for (const AActor* LandscapeActor : LandscapeActors)
	{
		FNRawMesh LandscapeMesh;
		if (!SampleLandscapeSurface(LandscapeActor, LandscapeSampleSpacing, LandscapeMesh)) continue;

		LandscapeVertexCount += LandscapeMesh.Vertices.Num();
		for (const FVector& Vertex : LandscapeMesh.Vertices)
		{
			Vertices.Add(Chaos::FConvex::FVec3Type(Vertex));
		}
	}

	// Census of what the hull builder is about to be handed, and of how much the terrain thinning saved it. The first
	// thing to look at when a hull calculation goes slow, or when a hull fails to appear at all.
	UE_LOG(LogNexusWorldAssembly, Log,
		TEXT("CalculateConvexHull source geometry: %d actors (%d terrain), %d meshes, %d vertices, %d triangles; terrain %d vertices thinned to %d; %d landscape samples."),
		HullActors.Num() + TerrainActors.Num() + LandscapeActors.Num(), TerrainActorCount,
		CollisionMeshes.Num() + TerrainMeshes.Num(), CollisionVertexCount + TerrainVertexCount, CollisionTriangleCount,
		TerrainVertexCount, VerticesAfterTerrain - VerticesBeforeTerrain, LandscapeVertexCount);

	// STEP 3 - Non-colliding actors yield no collision geometry, so (when requested) fall back to their
	// bounding-box corners. A registered primitive with a BodySetup is the same gate FNRawMeshFactory uses
	// to emit geometry, so its absence means the factory produced nothing for this actor.
	if (Settings.bIncludeNonColliding)
	{
		FVector BoxVertices[8];
		for (AActor* Actor : HullActors)
		{
			bool bHasCollisionGeometry = false;
			TInlineComponentArray<UPrimitiveComponent*> ActorPrimitives(Actor);
			for (UPrimitiveComponent* ActorPrimitive : ActorPrimitives)
			{
				if (ActorPrimitive != nullptr && ActorPrimitive->IsRegistered() && ActorPrimitive->GetBodySetup() != nullptr)
				{
					bHasCollisionGeometry = true;
					break;
				}
			}
			if (bHasCollisionGeometry) continue;

			FBox ActorBox = Actor->GetComponentsBoundingBox(true);
			if (ActorBox.IsValid &&
				(ActorBox.GetExtent().X > 0 && ActorBox.GetExtent().Y > 0 && ActorBox.GetExtent().Z > 0))
			{
				ActorBox.GetVertices(BoxVertices);
				for (const FVector& BoxVertex : BoxVertices)
				{
					Vertices.Add(Chaos::FConvex::FVec3Type(BoxVertex));
				}
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

bool FNWorldAssemblyUtils::SampleLandscapeSurface(const AActor* LandscapeActor, const double GridSize, FNRawMesh& OutMesh)
{
	if (!IsValid(LandscapeActor) || GridSize <= 0.0) return false;

	const UWorld* World = LandscapeActor->GetWorld();
	if (World == nullptr) return false;

	const FBox Bounds = LandscapeActor->GetComponentsBoundingBox(true);
	if (!Bounds.IsValid) return false;

	const FVector Size = Bounds.GetSize();
	const int32 CountX = FMath::Max(1, FMath::CeilToInt32(Size.X / GridSize)) + 1;
	const int32 CountY = FMath::Max(1, FMath::CeilToInt32(Size.Y / GridSize)) + 1;

	// Clear of the surface at both ends so a trace can neither start inside the landscape nor stop short of a dip.
	constexpr double Margin = 100.0;
	const double TraceTop = Bounds.Max.Z + Margin;
	const double TraceBottom = Bounds.Min.Z - Margin;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(NSampleLandscapeSurface), true);

	// Multi rather than single: the first blocking hit down a column is whatever sits on the landscape, and taking
	// that would sample the props instead of the ground under them.
	TArray<FHitResult> Hits;
	TArray<FVector> Samples;
	TArray<bool> SampleValid;
	Samples.SetNum(CountX * CountY);
	SampleValid.SetNum(CountX * CountY);

	for (int32 IndexX = 0; IndexX < CountX; IndexX++)
	{
		for (int32 IndexY = 0; IndexY < CountY; IndexY++)
		{
			const double X = FMath::Min(Bounds.Min.X + IndexX * GridSize, Bounds.Max.X);
			const double Y = FMath::Min(Bounds.Min.Y + IndexY * GridSize, Bounds.Max.Y);
			const int32 SampleIndex = IndexX * CountY + IndexY;

			Hits.Reset();
			SampleValid[SampleIndex] = false;

			if (!World->LineTraceMultiByChannel(Hits, FVector(X, Y, TraceTop), FVector(X, Y, TraceBottom), ECC_WorldStatic, Params))
			{
				continue;
			}

			for (const FHitResult& Hit : Hits)
			{
				if (Hit.GetActor() != LandscapeActor) continue;

				Samples[SampleIndex] = Hit.ImpactPoint;
				SampleValid[SampleIndex] = true;
				break;
			}
		}
	}

	// Emit a quad only where all four of its corners found the surface, so a hole in the landscape leaves a hole here
	// rather than a triangle stretched across it.
	OutMesh = FNRawMesh();
	TMap<int32, int32> SampleToVertex;
	auto AddVertex = [&OutMesh, &SampleToVertex, &Samples](const int32 SampleIndex)
	{
		if (const int32* Existing = SampleToVertex.Find(SampleIndex)) return *Existing;

		const int32 NewIndex = OutMesh.Vertices.Add(Samples[SampleIndex]);
		SampleToVertex.Add(SampleIndex, NewIndex);
		return NewIndex;
	};

	for (int32 IndexX = 0; IndexX < CountX - 1; IndexX++)
	{
		for (int32 IndexY = 0; IndexY < CountY - 1; IndexY++)
		{
			const int32 A = IndexX * CountY + IndexY;
			const int32 B = (IndexX + 1) * CountY + IndexY;
			const int32 C = (IndexX + 1) * CountY + (IndexY + 1);
			const int32 D = IndexX * CountY + (IndexY + 1);
			if (!SampleValid[A] || !SampleValid[B] || !SampleValid[C] || !SampleValid[D]) continue;

			const int32 VertexA = AddVertex(A);
			const int32 VertexB = AddVertex(B);
			const int32 VertexC = AddVertex(C);
			const int32 VertexD = AddVertex(D);

			OutMesh.Loops.Add(FNRawMeshLoop(VertexA, VertexB, VertexC));
			OutMesh.Loops.Add(FNRawMeshLoop(VertexA, VertexC, VertexD));
		}
	}

	if (OutMesh.Loops.IsEmpty()) return false;

	OutMesh.CalculateCenterAndBounds();
	OutMesh.Validate();
	return true;
}

FNCellVoxelData FNWorldAssemblyUtils::CalculateVoxelData(ULevel* InLevel, const FNCellVoxelGenerationSettings& Settings)
{
	// TODO: We probably could use the voxel data to actually generate the overall bounds to avoid the double parse of the actors in the level
	FNCellVoxelData ReturnData;

	// Go home early
	if (InLevel == nullptr)
	{
		return ReturnData;
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
	// The ignored-actor list this fills is handed to the sweep below, so one filter settles both the grid extents and
	// what the sweep is allowed to hit.
	FNLevelBoundsFilter Filter;
	Filter.ActorIgnoreTags = Settings.ActorIgnoreTags;
	Filter.bIncludeEditorOnly = Settings.bIncludeEditorOnly;
	Filter.bIncludeNonColliding = Settings.bIncludeNonColliding;
	Filter.bIncludeTerrain = Settings.bIncludeTerrain;

	FBox Bounds(ForceInit);
	FNLevelUtils::DetermineLevelBounds(InLevel, Bounds, IgnoredActors, Filter);

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

	return ReturnData;
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

	return Result;

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
	return Points;
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

	return ReturnPositions;
}

TArray<FVector> FNWorldAssemblyUtils::GetJunctionWorldCornerPoints(const FVector& Location, const FRotator& Rotation,
	const FIntVector2& Units, const FVector2D& UnitSize)
{
	// Compose: yaw-align the local XZ-plane corners into the YZ-plane (rect normal becomes +X), then apply the
	// junction rotation. Rotator addition is component-wise and only matches composition for yaw-only inputs;
	// quaternions compose correctly for pitch and roll too.
	const FQuat DisplayQuat = FQuat(Rotation) * FQuat(FRotator(0.0f, 90.0f, 0.0f));

	const FVector2D Size = GetWorldSize2D(Units, UnitSize);
	const TArray<FVector> UnrotatedCornerPoints = GetCenteredWorldCornerPoints2D(Size.X, Size.Y, ENAxis::Z);
	return FNVectorUtils::RotateAndOffsetPoints(UnrotatedCornerPoints, DisplayQuat.Rotator(), Location);
}

TArray<FVector> FNWorldAssemblyUtils::GetJunctionWorldCornerPoints(const FNCellJunctionDetails& Details, const FVector2D& UnitSize)
{
	return GetJunctionWorldCornerPoints(Details.WorldLocation, Details.WorldRotation, Details.SocketSize, UnitSize);
}

bool FNWorldAssemblyUtils::AreJunctionsInverseCoincident(const FNCellJunctionDetails& A, const FNCellJunctionDetails& B,
	const FVector2D& UnitSize, const float Tolerance)
{
	// Cheap rejections first: both are a single comparison, and between them they discard nearly everything a
	// proximity broadphase hands over.
	if (A.SocketSize != B.SocketSize) return false;
	if (!A.WorldLocation.Equals(B.WorldLocation, Tolerance)) return false;

	// Opposed, not merely parallel. Two junctions in the same place opening onto the *same* direction are two cells
	// stacked back to back, and their socket rectangles coincide just as neatly — so without this the corner test
	// below would happily accept them. Only the sign matters: coincident rectangles share a plane, which already
	// pins the normals to parallel or antiparallel.
	if (FVector::DotProduct(GetJunctionOutwardDirection(A), GetJunctionOutwardDirection(B)) >= 0.0) return false;

	const TArray<FVector> CornersA = GetJunctionWorldCornerPoints(A, UnitSize);
	const TArray<FVector> CornersB = GetJunctionWorldCornerPoints(B, UnitSize);
	if (CornersA.Num() != 4 || CornersB.Num() != 4) return false;

	// Every corner of A must claim a distinct corner of B. The two rectangles are built in opposite winding order, so
	// there is no index correspondence to rely on; claiming by mask is what stops one corner of B standing in for two
	// of A. Greedy claiming is exact here rather than merely approximate, because a socket's corners are separated by
	// at least its shortest side and the tolerance is far below that — no corner is ever ambiguous between two
	// partners.
	uint8 ClaimedMask = 0;
	for (const FVector& Corner : CornersA)
	{
		bool bClaimed = false;
		for (int32 i = 0; i < 4; i++)
		{
			if ((ClaimedMask & 1 << i) != 0) continue;
			if (!Corner.Equals(CornersB[i], Tolerance)) continue;

			ClaimedMask |= 1 << i;
			bClaimed = true;
			break;
		}

		if (!bClaimed) return false;
	}

	return true;
}

bool FNWorldAssemblyUtils::AreJunctionsWithinConnectionAngles(const FNCellJunctionDetails& A, const FNCellJunctionDetails& B,
	const float DefaultMaximumFacingAngle, const float DefaultMaximumApproachAngle, const float DefaultMaximumElevationDifference)
{
	// Resolve each end's limits up front. An opted-in junction supplies its own; where a test spans the pair the
	// stricter of the two is taken, which is what stops a permissive override loosening a strict partner.
	const FNCellJunctionConnectionConstraints& ConstraintsA = A.ConnectionConstraints;
	const FNCellJunctionConnectionConstraints& ConstraintsB = B.ConnectionConstraints;

	const float ApproachLimitA = ConstraintsA.bOverrideAngleLimits ? ConstraintsA.MaximumApproachAngle : DefaultMaximumApproachAngle;
	const float ApproachLimitB = ConstraintsB.bOverrideAngleLimits ? ConstraintsB.MaximumApproachAngle : DefaultMaximumApproachAngle;
	const float FacingLimit = FMath::Min(
		ConstraintsA.bOverrideAngleLimits ? ConstraintsA.MaximumFacingAngle : DefaultMaximumFacingAngle,
		ConstraintsB.bOverrideAngleLimits ? ConstraintsB.MaximumFacingAngle : DefaultMaximumFacingAngle);
	const float ElevationLimit = FMath::Min(
		ConstraintsA.bOverrideAngleLimits ? ConstraintsA.MaximumElevationDifference : DefaultMaximumElevationDifference,
		ConstraintsB.bOverrideAngleLimits ? ConstraintsB.MaximumElevationDifference : DefaultMaximumElevationDifference);

	const FVector OutwardA = GetJunctionOutwardDirection(A);
	const FVector OutwardB = GetJunctionOutwardDirection(B);

	// Compared as cosines rather than angles: a dot product against one precomputed cosine settles each test, where
	// converting back would spend an inverse trig call per candidate pair. Cosine falls as an angle opens, so a
	// wider angle is a smaller dot and the comparison inverts. The slack keeps a limit landing exactly on its
	// boundary — 90 degrees against a perfectly perpendicular pair — inclusive, matching how the limits read.
	if (FVector::DotProduct(OutwardA, -OutwardB) < FMath::Cos(FMath::DegreesToRadians(FacingLimit)) - UE_KINDA_SMALL_NUMBER)
	{
		return false;
	}

	// Coincident sockets have no line between them to measure against. Skipping rather than rejecting leaves the
	// pair to the solver, which reports it degenerate; inverse matching, which is where such a pair belongs, runs
	// ahead of this and never asks.
	const FVector Chord = B.WorldLocation - A.WorldLocation;
	const double ChordLengthSquared = Chord.SizeSquared();
	if (ChordLengthSquared > UE_DOUBLE_SMALL_NUMBER)
	{
		const FVector ChordDirection = Chord * (1.0 / FMath::Sqrt(ChordLengthSquared));

		if (FVector::DotProduct(OutwardA, ChordDirection) < FMath::Cos(FMath::DegreesToRadians(ApproachLimitA)) - UE_KINDA_SMALL_NUMBER)
		{
			return false;
		}
		if (FVector::DotProduct(OutwardB, -ChordDirection) < FMath::Cos(FMath::DegreesToRadians(ApproachLimitB)) - UE_KINDA_SMALL_NUMBER)
		{
			return false;
		}
	}

	// Elevation is the only one of the three that cannot collapse to a dot product: it compares two angles measured
	// against the horizontal rather than the angle between two directions, so the difference has to be taken in
	// angle space. Both outward directions are unit length, which makes the Z component the sine of the elevation
	// directly. Left until last so the two dot-product tests above reject first.
	const double ElevationA = FMath::Asin(FMath::Clamp(OutwardA.Z, -1.0, 1.0));
	const double ElevationB = FMath::Asin(FMath::Clamp(OutwardB.Z, -1.0, 1.0));
	return FMath::Abs(ElevationA - ElevationB) <= FMath::DegreesToRadians(ElevationLimit) + UE_KINDA_SMALL_NUMBER;
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
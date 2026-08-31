// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldCollisionPool.h"

#include "NWorldAssemblyMinimal.h"

void FNWorldCollisionPool::SerializeMesh(FArchive& Ar, FNRawMesh& Mesh)
{
	Ar << Mesh.Vertices;
	Ar << Mesh.Center;
	Ar << Mesh.Bounds;

	int32 LoopCount = Mesh.Loops.Num();
	Ar << LoopCount;
	if (Ar.IsLoading())
	{
		Mesh.Loops.SetNum(LoopCount);
	}
	for (int32 i = 0; i < LoopCount; ++i)
	{
		Ar << Mesh.Loops[i].Indices;
	}

	int32 FaceLoopCount = Mesh.FaceLoops.Num();
	Ar << FaceLoopCount;
	if (Ar.IsLoading())
	{
		Mesh.FaceLoops.SetNum(FaceLoopCount);
	}
	for (int32 i = 0; i < FaceLoopCount; ++i)
	{
		Ar << Mesh.FaceLoops[i].Indices;
	}

	Ar << Mesh.bIsConvex;
	Ar << Mesh.bIsChaosGenerated;
	Ar << Mesh.bHasNonTris;
	Ar << Mesh.bHasBounds;
	Ar << Mesh.bHasAppliedTransform;
}

void FNWorldCollisionPool::SerializeMeshArray(FArchive& Ar, TArray<FNRawMesh>& Meshes)
{
	int32 Count = Meshes.Num();
	Ar << Count;
	if (Ar.IsLoading())
	{
		Meshes.Empty(Count);
		Meshes.SetNum(Count);
	}
	for (int32 i = 0; i < Count; ++i)
	{
		SerializeMesh(Ar, Meshes[i]);
	}
}

void FNWorldCollisionPool::Reset()
{
	Keys.Reset();
	Meshes.Reset();
	LandscapeMeshes.Reset();
	LandscapeFingerprint = 0;
	BakeOriginLocation = FIntVector::ZeroValue;
	KeyIndex.Reset();
	bKeyIndexValid = false;
}

void FNWorldCollisionPool::EnsureIndex() const
{
	if (bKeyIndexValid) return;

	KeyIndex.Reset();
	KeyIndex.Reserve(Keys.Num());
	for (int32 i = 0; i < Keys.Num(); ++i)
	{
		KeyIndex.Add(Keys[i], i);
	}
	bKeyIndexValid = true;
}

bool FNWorldCollisionPool::Contains(const uint64 Key) const
{
	EnsureIndex();
	return KeyIndex.Contains(Key);
}

void FNWorldCollisionPool::Add(const uint64 Key, FNRawMesh&& Mesh)
{
	EnsureIndex();

	if (const int32* Existing = KeyIndex.Find(Key))
	{
		Meshes[*Existing] = MoveTemp(Mesh);
		return;
	}

	const int32 NewIndex = Keys.Add(Key);
	Meshes.Add(MoveTemp(Mesh));
	KeyIndex.Add(Key, NewIndex);
}

bool FNWorldCollisionPool::Resolve(const TArray<uint64>& InKeys, TArray<FNRawMesh>& OutMeshes) const
{
	EnsureIndex();

	OutMeshes.Reserve(OutMeshes.Num() + InKeys.Num());
	for (const uint64 Key : InKeys)
	{
		const int32* Index = KeyIndex.Find(Key);
		if (Index == nullptr)
		{
			return false;
		}
		OutMeshes.Add(Meshes[*Index]);
	}
	return true;
}

int32 FNWorldCollisionPool::RemoveUnreferenced(const TSet<uint64>& Referenced)
{
	const int32 CountBefore = Keys.Num();

	// Compacted in one pass rather than by repeated RemoveAt, which would be quadratic across a level-sized pool.
	int32 Write = 0;
	for (int32 Read = 0; Read < Keys.Num(); ++Read)
	{
		if (!Referenced.Contains(Keys[Read])) continue;

		if (Write != Read)
		{
			Keys[Write] = Keys[Read];
			Meshes[Write] = MoveTemp(Meshes[Read]);
		}
		Write++;
	}

	Keys.SetNum(Write);
	Meshes.SetNum(Write);
	bKeyIndexValid = false;

	return CountBefore - Write;
}

bool FNWorldCollisionPool::Identical(const FNWorldCollisionPool* Other, uint32 PortFlags) const
{
	if (Other == nullptr) return false;

	// Cheap scalars and counts first. Against the empty default — which is what the save-time comparison actually
	// asks about — a real pool differs at the very first of these and never reaches the geometry walk below.
	if (LandscapeFingerprint != Other->LandscapeFingerprint) return false;
	if (BakeOriginLocation != Other->BakeOriginLocation) return false;
	if (Keys != Other->Keys) return false;
	if (Meshes.Num() != Other->Meshes.Num()) return false;
	if (LandscapeMeshes.Num() != Other->LandscapeMeshes.Num()) return false;

	// Matching keys do not imply matching geometry: an actor that moved keeps its key and changes its hull, and
	// stopping at the key list would call that pool unchanged and decline to save the new geometry.
	for (int32 i = 0; i < Meshes.Num(); ++i)
	{
		if (!(Meshes[i] == Other->Meshes[i])) return false;
	}
	for (int32 i = 0; i < LandscapeMeshes.Num(); ++i)
	{
		if (!(LandscapeMeshes[i] == Other->LandscapeMeshes[i])) return false;
	}

	return true;
}

bool FNWorldCollisionPool::Serialize(FArchive& Ar)
{
	uint8 Version = CurrentVersion;
	Ar << Version;

	if (Ar.IsLoading() && Version != CurrentVersion)
	{
		// A pool written by a different build of the format. Discarded rather than guessed at: every consumer treats
		// an empty pool as "nothing cached" and gathers afresh, so the cost of being wrong here is one slow assembly
		// and a rebake on the next save.
		UE_LOG(LogNexusWorldAssembly, Log,
			TEXT("Discarding world collision pool written at version %u; this build reads version %u."),
			Version, CurrentVersion);
		Reset();
		return true;
	}

	Ar << Keys;
	SerializeMeshArray(Ar, Meshes);
	SerializeMeshArray(Ar, LandscapeMeshes);
	Ar << LandscapeFingerprint;
	Ar << BakeOriginLocation;

	if (Ar.IsLoading())
	{
		bKeyIndexValid = false;

		// Keys and Meshes are written as independent arrays, so a truncated or hand-edited payload can desynchronize
		// them. Nothing downstream re-checks this, and a mismatched pair would index one array with the other's
		// bounds, so it is settled here.
		if (Keys.Num() != Meshes.Num())
		{
			UE_LOG(LogNexusWorldAssembly, Warning,
				TEXT("World collision pool loaded with %d keys against %d meshes; discarding it."),
				Keys.Num(), Meshes.Num());
			Reset();
		}
	}

	return true;
}

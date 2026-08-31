// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldCollisionPool.h"
#include "Macros/NTestMacros.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPoolHarness
{
	/** A minimal but well-formed tetrahedron, offset so that meshes built with different seeds are distinguishable. */
	static FNRawMesh MakeMesh(const double Offset)
	{
		FNRawMesh Mesh;
		Mesh.Vertices = {
			{ Offset, 0.0, 0.0 },
			{ Offset + 100.0, 0.0, 0.0 },
			{ Offset, 100.0, 0.0 },
			{ Offset, 0.0, 100.0 },
		};
		Mesh.Loops.Add(FNRawMeshLoop(0, 2, 1));
		Mesh.Loops.Add(FNRawMeshLoop(0, 1, 3));
		Mesh.Loops.Add(FNRawMeshLoop(0, 3, 2));
		Mesh.Loops.Add(FNRawMeshLoop(1, 2, 3));
		Mesh.Center = FVector(Offset, 0.0, 0.0);
		Mesh.Bounds = FBox(FVector(Offset, 0.0, 0.0), FVector(Offset + 100.0, 100.0, 100.0));
		return Mesh;
	}

	/** A pool holding three elements under the supplied keys. */
	static FNWorldCollisionPool MakePool()
	{
		FNWorldCollisionPool Pool;
		Pool.Add(10, MakeMesh(0.0));
		Pool.Add(20, MakeMesh(1000.0));
		Pool.Add(30, MakeMesh(2000.0));
		return Pool;
	}
}

N_TEST_HIGH(FNWorldCollisionPoolTests_Add_StoresAndFindsByKey,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPool::Add::StoresAndFindsByKey",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPoolHarness;

	const FNWorldCollisionPool Pool = MakePool();

	CHECK_EQUALS("Three added elements must be held.", Pool.Num(), 3);
	CHECK_MESSAGE(TEXT("A stored key must be found."), Pool.Contains(20));
	CHECK_FALSE_MESSAGE(TEXT("A key that was never added must not be found."), Pool.Contains(999));
}

N_TEST_HIGH(FNWorldCollisionPoolTests_Add_ReplacesInPlaceForAKnownKey,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPool::Add::ReplacesInPlaceForAKnownKey",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPoolHarness;

	// Re-baking one changed element must update it, not append a second entry under the same key — a duplicate would
	// resolve twice and be collision-tested twice for the rest of the pool's life.
	FNWorldCollisionPool Pool = MakePool();
	Pool.Add(20, MakeMesh(5000.0));

	CHECK_EQUALS("Replacing a key must not grow the pool.", Pool.Num(), 3);

	TArray<FNRawMesh> Resolved;
	Pool.Resolve({ 20 }, Resolved);
	CHECK_EQUALS("The replacement geometry must be what resolves.", Resolved[0].Center.X, 5000.0);
}

N_TEST_CRITICAL(FNWorldCollisionPoolTests_Resolve_FailsOnAMissingKey,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPool::Resolve::FailsOnAMissingKey",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPoolHarness;

	// This is the failure mode the whole key-over-index design exists to make detectable: an organ referencing
	// geometry the pool no longer holds must report a miss, never silently resolve to something else.
	const FNWorldCollisionPool Pool = MakePool();

	TArray<FNRawMesh> Resolved;
	const bool bResolved = Pool.Resolve({ 10, 999 }, Resolved);

	CHECK_FALSE_MESSAGE(TEXT("Resolving a key the pool does not hold must fail."), bResolved);
}

N_TEST_HIGH(FNWorldCollisionPoolTests_Resolve_AppendsInRequestedOrder,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPool::Resolve::AppendsInRequestedOrder",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPoolHarness;

	const FNWorldCollisionPool Pool = MakePool();

	TArray<FNRawMesh> Resolved;
	const bool bResolved = Pool.Resolve({ 30, 10 }, Resolved);

	CHECK_MESSAGE(TEXT("Every requested key resolves."), bResolved);
	CHECK_EQUALS("Two keys resolve to two meshes.", Resolved.Num(), 2);
	CHECK_EQUALS("The first requested key resolves first.", Resolved[0].Center.X, 2000.0);
	CHECK_EQUALS("The second requested key resolves second.", Resolved[1].Center.X, 0.0);
}

N_TEST_HIGH(FNWorldCollisionPoolTests_RemoveUnreferenced_CompactsAndKeepsLookupCorrect,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPool::RemoveUnreferenced::CompactsAndKeepsLookupCorrect",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPoolHarness;

	// Orphan collection: an element no organ names any more is dropped, and the survivors must still resolve to
	// their own geometry rather than to whatever shifted into their old slot.
	FNWorldCollisionPool Pool = MakePool();
	const int32 Removed = Pool.RemoveUnreferenced(TSet<uint64>{ 10, 30 });

	CHECK_EQUALS("One unreferenced element must be removed.", Removed, 1);
	CHECK_EQUALS("Two referenced elements must remain.", Pool.Num(), 2);
	CHECK_FALSE_MESSAGE(TEXT("The unreferenced key must be gone."), Pool.Contains(20));

	TArray<FNRawMesh> Resolved;
	const bool bResolved = Pool.Resolve({ 30, 10 }, Resolved);
	CHECK_MESSAGE(TEXT("Surviving keys must still resolve after compaction."), bResolved);
	CHECK_EQUALS("Compaction must not shuffle geometry between keys.", Resolved[0].Center.X, 2000.0);
	CHECK_EQUALS("Compaction must not shuffle geometry between keys.", Resolved[1].Center.X, 0.0);
}

N_TEST_CRITICAL(FNWorldCollisionPoolTests_Identical_DiffersFromAnEmptyPool,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPool::Identical::DiffersFromAnEmptyPool",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPoolHarness;

	// What decides whether the pool reaches disk. UE declines to write a property equal to its default, and nothing in
	// this struct is reflected — so without a hand-written Identical every baked level saves an empty pool.
	const FNWorldCollisionPool Empty;
	const FNWorldCollisionPool Filled = MakePool();

	CHECK_FALSE_MESSAGE(TEXT("A filled pool must not compare identical to an empty one, or it is never saved."),
		Filled.Identical(&Empty, 0));
	CHECK_MESSAGE(TEXT("A pool must compare identical to a copy of itself."), Filled.Identical(&Filled, 0));
	CHECK_FALSE_MESSAGE(TEXT("Null is never identical."), Filled.Identical(nullptr, 0));
}

N_TEST_HIGH(FNWorldCollisionPoolTests_Identical_DetectsGeometryChangeUnderTheSameKeys,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPool::Identical::DetectsGeometryChangeUnderTheSameKeys",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPoolHarness;

	// The case a key-list comparison alone would miss: an actor that moved keeps its key and changes its hull. Calling
	// that pool unchanged would decline to save the new geometry while the keys said everything was fine.
	FNWorldCollisionPool Before = MakePool();
	FNWorldCollisionPool After = MakePool();
	After.Add(20, MakeMesh(9999.0));

	CHECK_MESSAGE(TEXT("Both pools must still hold the same keys, or this is testing the wrong thing."),
		Before.Num() == After.Num() && After.Contains(20));
	CHECK_FALSE_MESSAGE(TEXT("Changed geometry under an unchanged key must not compare identical."),
		Before.Identical(&After, 0));
}

N_TEST_HIGH(FNWorldCollisionPoolTests_Identical_DetectsLandscapeAndOriginChanges,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPool::Identical::DetectsLandscapeAndOriginChanges",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPoolHarness;

	const FNWorldCollisionPool Baseline = MakePool();

	FNWorldCollisionPool DifferentLandscape = MakePool();
	DifferentLandscape.LandscapeFingerprint = 0x77ULL;
	CHECK_FALSE_MESSAGE(TEXT("A differing landscape fingerprint must not compare identical."),
		Baseline.Identical(&DifferentLandscape, 0));

	FNWorldCollisionPool DifferentOrigin = MakePool();
	DifferentOrigin.BakeOriginLocation = FIntVector(1, 0, 0);
	CHECK_FALSE_MESSAGE(TEXT("A differing bake origin must not compare identical."),
		Baseline.Identical(&DifferentOrigin, 0));

	FNWorldCollisionPool ExtraLandscapeMesh = MakePool();
	ExtraLandscapeMesh.LandscapeMeshes.Add(MakeMesh(0.0));
	CHECK_FALSE_MESSAGE(TEXT("A differing landscape section must not compare identical."),
		Baseline.Identical(&ExtraLandscapeMesh, 0));
}

N_TEST_CRITICAL(FNWorldCollisionPoolTests_Serialize_RoundTripsBothSections,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPool::Serialize::RoundTripsBothSections",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPoolHarness;

	FNWorldCollisionPool Written = MakePool();
	Written.LandscapeMeshes.Add(MakeMesh(7000.0));
	Written.LandscapeFingerprint = 0xABCDEF0123456789ULL;
	Written.BakeOriginLocation = FIntVector(1000, 2000, 3000);

	TArray<uint8> Bytes;
	FMemoryWriter Writer(Bytes);
	Written.Serialize(Writer);

	FNWorldCollisionPool Read;
	FMemoryReader Reader(Bytes);
	Read.Serialize(Reader);

	CHECK_EQUALS("Every keyed element must survive the round trip.", Read.Num(), 3);
	CHECK_MESSAGE(TEXT("Keys must survive the round trip."), Read.Contains(10) && Read.Contains(20) && Read.Contains(30));
	CHECK_EQUALS("The landscape section must survive the round trip.", Read.LandscapeMeshes.Num(), 1);
	CHECK_MESSAGE(TEXT("The landscape fingerprint must survive the round trip."),
		Read.LandscapeFingerprint == 0xABCDEF0123456789ULL);
	CHECK_MESSAGE(TEXT("The bake origin must survive the round trip."),
		Read.BakeOriginLocation == FIntVector(1000, 2000, 3000));

	// Geometry, not just bookkeeping: the whole point of the pool is that the loaded hulls are usable as-is.
	TArray<FNRawMesh> Resolved;
	const bool bResolved = Read.Resolve({ 20 }, Resolved);
	CHECK_MESSAGE(TEXT("A loaded pool must resolve its keys."), bResolved);
	CHECK_EQUALS("Vertices must survive the round trip.", Resolved[0].Vertices.Num(), 4);
	CHECK_EQUALS("Loops must survive the round trip.", Resolved[0].Loops.Num(), 4);
	CHECK_EQUALS("Loop indices must survive the round trip.", Resolved[0].Loops[3].Indices.Num(), 3);
	CHECK_EQUALS("Geometry must land under the key it was stored against.", Resolved[0].Center.X, 1000.0);
}

N_TEST_HIGH(FNWorldCollisionPoolTests_Reset_EmptiesEverything,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPool::Reset::EmptiesEverything",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPoolHarness;

	FNWorldCollisionPool Pool = MakePool();
	Pool.LandscapeMeshes.Add(MakeMesh(0.0));
	Pool.Reset();

	CHECK_MESSAGE(TEXT("A reset pool must report empty."), Pool.IsEmpty());
	CHECK_FALSE_MESSAGE(TEXT("A reset pool must not resolve a key it used to hold."), Pool.Contains(10));
}

#endif //WITH_TESTS

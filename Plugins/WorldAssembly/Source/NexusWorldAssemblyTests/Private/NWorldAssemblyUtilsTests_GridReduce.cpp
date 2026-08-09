// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldAssemblyUtils.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

/**
 * Grid thinning of terrain vertices ahead of the convex build.
 *
 * The property worth pinning is not the count reduction — that is obvious — but the direction of the error. Each kept
 * point is snapped away from the source geometry's center, so the envelope it produces errs outward rather than
 * cutting inside the surface it stands for. A thinning that snapped the other way would quietly shrink every terrain
 * hull by up to a grid cell, which is exactly the kind of thing that goes unnoticed until cells intersect terrain.
 */

N_TEST_HIGH(FNWorldAssemblyUtilsTests_GridReduce_SnapsOutward,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::GridReduce::SnapsOutward",
	N_TEST_CONTEXT_ANYWHERE)
{
	// One point per octant around the origin, none of them on a grid line.
	const TArray<FVector> Points = {
		FVector(10.0, 10.0, 10.0), FVector(-10.0, -10.0, -10.0),
		FVector(10.0, -10.0, 10.0), FVector(-10.0, 10.0, -10.0)
	};

	TSet<FIntVector> SeenCells;
	TArray<FVector> Reduced;
	FNWorldAssemblyUtils::GridReducePoints(Points, FVector::ZeroVector, 100.0, SeenCells, Reduced);

	CHECK_EQUALS("Four points in four distinct cells are all kept.", Reduced.Num(), 4);

	// Every result must sit at or beyond its source on each axis, measured from the center.
	for (const FVector& Point : Reduced)
	{
		CHECK_MESSAGE(TEXT("A point right of center does not move left of where it started."),
			Point.X >= 10.0 - UE_KINDA_SMALL_NUMBER || Point.X <= -10.0 + UE_KINDA_SMALL_NUMBER);
		CHECK_MESSAGE(TEXT("The snap does not exceed one grid cell."), FMath::Abs(Point.X) <= 100.0 + UE_KINDA_SMALL_NUMBER);
	}
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_GridReduce_ErrorBoundedByGrid,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::GridReduce::ErrorBoundedByGrid",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A dense sweep, so cells land at every offset within the grid rather than at a convenient one.
	constexpr double GridSize = 50.0;
	TArray<FVector> Points;
	for (int32 Index = 0; Index < 400; Index++)
	{
		const double T = Index * 3.7;
		Points.Add(FVector(T, FMath::Sin(T) * 250.0, FMath::Cos(T) * 120.0));
	}

	FBox Bounds(ForceInit);
	for (const FVector& Point : Points)
	{
		Bounds += Point;
	}

	TSet<FIntVector> SeenCells;
	TArray<FVector> Reduced;
	FNWorldAssemblyUtils::GridReducePoints(Points, Bounds.GetCenter(), GridSize, SeenCells, Reduced);

	CHECK_MESSAGE(TEXT("Thinning removes points."), Reduced.Num() < Points.Num());
	CHECK_MESSAGE(TEXT("Thinning keeps at least one point."), Reduced.Num() > 0);

	// The envelope may grow by at most a cell on each side, and must not shrink.
	FBox ReducedBounds(ForceInit);
	for (const FVector& Point : Reduced)
	{
		ReducedBounds += Point;
	}

	CHECK_MESSAGE(TEXT("The thinned cloud does not cut inside the original on the low side."),
		ReducedBounds.Min.X <= Bounds.Min.X + UE_KINDA_SMALL_NUMBER
		&& ReducedBounds.Min.Y <= Bounds.Min.Y + UE_KINDA_SMALL_NUMBER
		&& ReducedBounds.Min.Z <= Bounds.Min.Z + UE_KINDA_SMALL_NUMBER);
	CHECK_MESSAGE(TEXT("The thinned cloud does not cut inside the original on the high side."),
		ReducedBounds.Max.X >= Bounds.Max.X - UE_KINDA_SMALL_NUMBER
		&& ReducedBounds.Max.Y >= Bounds.Max.Y - UE_KINDA_SMALL_NUMBER
		&& ReducedBounds.Max.Z >= Bounds.Max.Z - UE_KINDA_SMALL_NUMBER);

	CHECK_MESSAGE(TEXT("The thinned cloud grows by no more than one cell per side."),
		(Bounds.Min - ReducedBounds.Min).GetAbsMax() <= GridSize + UE_KINDA_SMALL_NUMBER
		&& (ReducedBounds.Max - Bounds.Max).GetAbsMax() <= GridSize + UE_KINDA_SMALL_NUMBER);
}

N_TEST_MEDIUM(FNWorldAssemblyUtilsTests_GridReduce_SharedGridAcrossMeshes,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::GridReduce::SharedGridAcrossMeshes",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Adjacent terrain sections share a seam, and their edge vertices coincide there. The cell set is carried across
	// calls precisely so the seam contributes once rather than once per section.
	const TArray<FVector> SectionA = { FVector(0.0, 0.0, 0.0), FVector(10.0, 0.0, 0.0) };
	const TArray<FVector> SectionB = { FVector(10.0, 0.0, 0.0), FVector(20.0, 0.0, 0.0) };

	TSet<FIntVector> SeenCells;
	TArray<FVector> Reduced;
	FNWorldAssemblyUtils::GridReducePoints(SectionA, FVector::ZeroVector, 100.0, SeenCells, Reduced);
	FNWorldAssemblyUtils::GridReducePoints(SectionB, FVector::ZeroVector, 100.0, SeenCells, Reduced);

	CHECK_EQUALS("All four points fall in one cell and yield a single kept point.", Reduced.Num(), 1);
}

N_TEST_MEDIUM(FNWorldAssemblyUtilsTests_GridReduce_DisabledKeepsEveryPoint,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::GridReduce::DisabledKeepsEveryPoint",
	N_TEST_CONTEXT_ANYWHERE)
{
	const TArray<FVector> Points = { FVector(1.0, 2.0, 3.0), FVector(1.5, 2.5, 3.5), FVector(2.0, 3.0, 4.0) };

	TSet<FIntVector> SeenCells;
	TArray<FVector> Reduced;
	FNWorldAssemblyUtils::GridReducePoints(Points, FVector::ZeroVector, 0.0, SeenCells, Reduced);

	CHECK_EQUALS("A grid size of zero keeps every point.", Reduced.Num(), Points.Num());
	CHECK_MESSAGE(TEXT("A grid size of zero leaves the points untouched."), Reduced[1].Equals(Points[1]));
}

N_TEST_MEDIUM(FNWorldAssemblyUtilsTests_GridReduce_EmptyInput,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::GridReduce::EmptyInput",
	N_TEST_CONTEXT_ANYWHERE)
{
	TSet<FIntVector> SeenCells;
	TArray<FVector> Reduced;
	FNWorldAssemblyUtils::GridReducePoints({}, FVector::ZeroVector, 100.0, SeenCells, Reduced);

	CHECK_EQUALS("An empty cloud yields nothing.", Reduced.Num(), 0);
}

#endif //WITH_TESTS

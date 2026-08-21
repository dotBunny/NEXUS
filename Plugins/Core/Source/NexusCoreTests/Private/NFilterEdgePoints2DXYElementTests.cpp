// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Macros/NTestMacros.h"
#include "PCG/Elements/NFilterEdgePoints2DXYElement.h"

namespace NEXUS::UnitTests::NCore::FNFilterEdgePoints2DXYElementHarness
{
	/** Spacing the harness grids are built on, matching the node's default. */
	static constexpr double GridSpacing = 100.0;

	/** Builds a filled Width x Height grid of XY positions on GridSpacing, laid out row by row. */
	static TArray<FVector> MakeGrid(const int32 Width, const int32 Height)
	{
		TArray<FVector> Positions;
		Positions.Reserve(Width * Height);
		for (int32 Y = 0; Y < Height; ++Y)
		{
			for (int32 X = 0; X < Width; ++X)
			{
				Positions.Emplace(X * GridSpacing, Y * GridSpacing, 0.0);
			}
		}

		return Positions;
	}

	/** Peels the supplied grid and renders the resulting ring indices, keeping each test's call site to one line. */
	static FString PeelAndDescribe(const TArray<FVector>& Positions, const int32 Depth)
	{
		// Brute-force stand-in for the element's octree query, using the same Spacing * 1.5 reach so the eight
		// surrounding neighbours are in and the next ring out is not.
		auto VisitNeighbors = [&Positions](const int32 Index, TFunctionRef<void(int32)> OnNeighbor)
		{
			const double SearchRadius = GridSpacing * 1.5;
			const double SquaredSearchRadius = SearchRadius * SearchRadius;
			for (int32 Other = 0; Other < Positions.Num(); ++Other)
			{
				if (Other != Index && FVector::DistSquaredXY(Positions[Index], Positions[Other]) <= SquaredSearchRadius)
				{
					OnNeighbor(Other);
				}
			}
		};

		TArray<int32> EdgeDepths;
		FNFilterEdgePoints2DXYElement::AssignEdgeDepths(Positions.Num(), Depth, VisitNeighbors, EdgeDepths);
		return FString::JoinBy(EdgeDepths, TEXT(","), [](const int32 EdgeDepth) { return FString::FromInt(EdgeDepth); });
	}
}

using namespace NEXUS::UnitTests::NCore::FNFilterEdgePoints2DXYElementHarness;

N_TEST_HIGH(FNFilterEdgePoints2DXYElementTests_AssignEdgeDepths_SingleDepthTakesTheOutlineOnly,
	"NEXUS::UnitTests::NCore::FNFilterEdgePoints2DXYElement::AssignEdgeDepths::SingleDepthTakesTheOutlineOnly",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The behaviour every existing graph relies on: the perimeter of a 5x5 grid comes off as ring one and the
	// 3x3 interior is left untouched. A depth of one has to stay identical to the plain edge test.
	const FString EdgeDepths = PeelAndDescribe(MakeGrid(5, 5), 1);
	CHECK_EQUALS("Only the perimeter should be taken.", *EdgeDepths,
		TEXT("1,1,1,1,1,")
		TEXT("1,0,0,0,1,")
		TEXT("1,0,0,0,1,")
		TEXT("1,0,0,0,1,")
		TEXT("1,1,1,1,1"));
}

N_TEST_HIGH(FNFilterEdgePoints2DXYElementTests_AssignEdgeDepths_SecondRingIsTheEdgeOfWhatRemains,
	"NEXUS::UnitTests::NCore::FNFilterEdgePoints2DXYElement::AssignEdgeDepths::SecondRingIsTheEdgeOfWhatRemains",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The whole point of the option: once the perimeter is gone, the 3x3 left behind has its own outline, and
	// only its centre still counts as interior.
	const FString EdgeDepths = PeelAndDescribe(MakeGrid(5, 5), 2);
	CHECK_EQUALS("The second ring should be the outline of the leftovers.", *EdgeDepths,
		TEXT("1,1,1,1,1,")
		TEXT("1,2,2,2,1,")
		TEXT("1,2,0,2,1,")
		TEXT("1,2,2,2,1,")
		TEXT("1,1,1,1,1"));
}

N_TEST_HIGH(FNFilterEdgePoints2DXYElementTests_AssignEdgeDepths_RingsNumberInwards,
	"NEXUS::UnitTests::NCore::FNFilterEdgePoints2DXYElement::AssignEdgeDepths::RingsNumberInwards",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A 7x7 grid is deep enough to carry three distinct rings, which is what the edge depth attribute has to
	// be able to tell apart downstream.
	const FString EdgeDepths = PeelAndDescribe(MakeGrid(7, 7), 3);
	CHECK_EQUALS("Each ring should carry its own index, rising inward.", *EdgeDepths,
		TEXT("1,1,1,1,1,1,1,")
		TEXT("1,2,2,2,2,2,1,")
		TEXT("1,2,3,3,3,2,1,")
		TEXT("1,2,3,0,3,2,1,")
		TEXT("1,2,3,3,3,2,1,")
		TEXT("1,2,2,2,2,2,1,")
		TEXT("1,1,1,1,1,1,1"));
}

N_TEST_MEDIUM(FNFilterEdgePoints2DXYElementTests_AssignEdgeDepths_DepthDeeperThanTheShapeConsumesIt,
	"NEXUS::UnitTests::NCore::FNFilterEdgePoints2DXYElement::AssignEdgeDepths::DepthDeeperThanTheShapeConsumesIt",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Asking for more rings than the input can supply has to run dry rather than spin, and leave nothing
	// behind on the interior pin.
	const FString EdgeDepths = PeelAndDescribe(MakeGrid(5, 5), 10);
	CHECK_EQUALS("Every point should be taken, none left as interior.", *EdgeDepths,
		TEXT("1,1,1,1,1,")
		TEXT("1,2,2,2,1,")
		TEXT("1,2,3,2,1,")
		TEXT("1,2,2,2,1,")
		TEXT("1,1,1,1,1"));
}

N_TEST_MEDIUM(FNFilterEdgePoints2DXYElementTests_AssignEdgeDepths_GridWithNoInteriorIsAllEdge,
	"NEXUS::UnitTests::NCore::FNFilterEdgePoints2DXYElement::AssignEdgeDepths::GridWithNoInteriorIsAllEdge",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Nothing in a two-row grid can reach eight neighbours, so there is no interior for a second ring to come
	// off; the first ring already accounts for the whole input.
	const FString EdgeDepths = PeelAndDescribe(MakeGrid(3, 2), 4);
	CHECK_EQUALS("A grid too thin to have an interior should come off in one ring.", *EdgeDepths, TEXT("1,1,1,1,1,1"));
}

N_TEST_MEDIUM(FNFilterEdgePoints2DXYElementTests_AssignEdgeDepths_DepthBelowOneFallsBackToOne,
	"NEXUS::UnitTests::NCore::FNFilterEdgePoints2DXYElement::AssignEdgeDepths::DepthBelowOneFallsBackToOne",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The property is clamped in the editor, but an override can still drive it below one. Taking no rings at
	// all would quietly turn the node into a passthrough, so it has to land on the single-ring case.
	const FString EdgeDepths = PeelAndDescribe(MakeGrid(5, 5), 0);
	CHECK_EQUALS("A depth of zero should behave as a depth of one.", *EdgeDepths,
		TEXT("1,1,1,1,1,")
		TEXT("1,0,0,0,1,")
		TEXT("1,0,0,0,1,")
		TEXT("1,0,0,0,1,")
		TEXT("1,1,1,1,1"));
}

N_TEST_MEDIUM(FNFilterEdgePoints2DXYElementTests_AssignEdgeDepths_EmptyInputProducesNoDepths,
	"NEXUS::UnitTests::NCore::FNFilterEdgePoints2DXYElement::AssignEdgeDepths::EmptyInputProducesNoDepths",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The element skips empty inputs, but the peel is public and must not assume a populated set.
	TArray<int32> EdgeDepths;
	EdgeDepths.Add(7);
	auto VisitNeighbors = [](int32, TFunctionRef<void(int32)>) {};
	FNFilterEdgePoints2DXYElement::AssignEdgeDepths(0, 3, VisitNeighbors, EdgeDepths);
	CHECK_EQUALS("An empty set should clear the output rather than leaving stale entries.", EdgeDepths.Num(), 0);
}

#endif //WITH_TESTS

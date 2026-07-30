// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Elements/NSortLine2DXYElement.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElementHarness
{
	/** The production deadzone, so tests exercise the same classification thresholds as the element. */
	constexpr float Tolerance = NEXUS::WorldAssembly::SortLine::TurnDeadzone;

	/** Part types as ints, matching ENSortLinePart, for terse expected-array literals. */
	constexpr int32 Wall = static_cast<int32>(ENSortLinePart::Wall);
	constexpr int32 Left = static_cast<int32>(ENSortLinePart::Left90);
	constexpr int32 Right = static_cast<int32>(ENSortLinePart::Right90);

	static TArray<int32> Parts(const TArray<FNSortLinePointInfo>& Info)
	{
		TArray<int32> Out;
		Out.Reserve(Info.Num());
		for (const FNSortLinePointInfo& Point : Info) Out.Add(static_cast<int32>(Point.Part));
		return Out;
	}

	static TArray<int32> SegmentIndices(const TArray<FNSortLinePointInfo>& Info)
	{
		TArray<int32> Out;
		Out.Reserve(Info.Num());
		for (const FNSortLinePointInfo& Point : Info) Out.Add(Point.SegmentIndex);
		return Out;
	}

	static TArray<int32> SegmentLengths(const TArray<FNSortLinePointInfo>& Info)
	{
		TArray<int32> Out;
		Out.Reserve(Info.Num());
		for (const FNSortLinePointInfo& Point : Info) Out.Add(Point.SegmentLength);
		return Out;
	}

	static TArray<int32> SubsegmentIndices(const TArray<FNSortLinePointInfo>& Info)
	{
		TArray<int32> Out;
		Out.Reserve(Info.Num());
		for (const FNSortLinePointInfo& Point : Info) Out.Add(Point.SubsegmentIndex);
		return Out;
	}

	/** @return true if every point reports a segment length of at least 1 (no point owns an empty segment). */
	static bool AllSegmentsNonEmpty(const TArray<FNSortLinePointInfo>& Info)
	{
		for (const FNSortLinePointInfo& Point : Info)
		{
			if (Point.SegmentLength < 1) return false;
		}
		return true;
	}
}

//
// ClassifyLine — grid classification (the documented axis-aligned input where turns are exactly 0 / +-1)
//

N_TEST_HIGH(FNSortLine2DXYElementTests_ClassifyLine_AllWalls,
	"NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElement::ClassifyLine::AllWalls",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElementHarness;
	// A straight open line: every point is a wall in a single segment whose length is the point count.
	TArray<FNSortLinePointInfo> Info;
	FNSortLine2DXYElement::ClassifyLine({ 0.f, 0.f, 0.f, 0.f }, Tolerance, Info);

	CHECK_MESSAGE(TEXT("A straight line should classify every point as a wall."),
		Parts(Info) == TArray<int32>({ Wall, Wall, Wall, Wall }))
	CHECK_MESSAGE(TEXT("Every wall on a straight line shares segment 0."),
		SegmentIndices(Info) == TArray<int32>({ 0, 0, 0, 0 }))
	CHECK_MESSAGE(TEXT("Every point should report the full run length as its segment length."),
		SegmentLengths(Info) == TArray<int32>({ 4, 4, 4, 4 }))
}

N_TEST_HIGH(FNSortLine2DXYElementTests_ClassifyLine_SingleCorner,
	"NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElement::ClassifyLine::SingleCorner",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElementHarness;
	// Open line with one left corner at index 2: walls split into two runs of length 2 either side of it.
	TArray<FNSortLinePointInfo> Info;
	FNSortLine2DXYElement::ClassifyLine({ 0.f, 0.f, 1.f, 0.f, 0.f }, Tolerance, Info);

	CHECK_MESSAGE(TEXT("A single positive turn should produce one left corner between two wall runs."),
		Parts(Info) == TArray<int32>({ Wall, Wall, Left, Wall, Wall }))
	CHECK_MESSAGE(TEXT("Segment indices should advance past the corner's own segment."),
		SegmentIndices(Info) == TArray<int32>({ 0, 0, 1, 2, 2 }))
	CHECK_MESSAGE(TEXT("Wall runs report length 2, the corner reports length 1."),
		SegmentLengths(Info) == TArray<int32>({ 2, 2, 1, 2, 2 }))
}

N_TEST_HIGH(FNSortLine2DXYElementTests_ClassifyLine_ConsecutiveCorners,
	"NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElement::ClassifyLine::ConsecutiveCorners",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElementHarness;
	// Adjacent corners (left then right) leave a zero-length gap segment between them that no point owns;
	// the bounds invariant must still hold and every point's reported length must be >= 1.
	TArray<FNSortLinePointInfo> Info;
	FNSortLine2DXYElement::ClassifyLine({ 0.f, 0.f, 1.f, -1.f, 0.f }, Tolerance, Info);

	CHECK_MESSAGE(TEXT("A +1 then -1 turn should produce a left corner immediately followed by a right corner."),
		Parts(Info) == TArray<int32>({ Wall, Wall, Left, Right, Wall }))
	CHECK_MESSAGE(TEXT("Each corner consumes its own segment index, leaving a gap between them."),
		SegmentIndices(Info) == TArray<int32>({ 0, 0, 1, 3, 4 }))
	CHECK_MESSAGE(TEXT("No point should resolve to the empty gap segment."),
		SegmentLengths(Info) == TArray<int32>({ 2, 2, 1, 1, 1 }))
	CHECK_MESSAGE(TEXT("Every point should own a non-empty segment."), AllSegmentsNonEmpty(Info))
}

N_TEST_HIGH(FNSortLine2DXYElementTests_ClassifyLine_CornerAtFirstIndex,
	"NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElement::ClassifyLine::CornerAtFirstIndex",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElementHarness;
	// A loop's closure turn lands at index 0; that corner must not try to close a preceding wall run.
	TArray<FNSortLinePointInfo> Info;
	FNSortLine2DXYElement::ClassifyLine({ 1.f, 0.f, 0.f, 0.f }, Tolerance, Info);

	CHECK_MESSAGE(TEXT("A turn at index 0 should classify the first point as a corner."),
		Parts(Info) == TArray<int32>({ Left, Wall, Wall, Wall }))
	CHECK_MESSAGE(TEXT("The index-0 corner owns segment 0; the trailing wall run is segment 1."),
		SegmentIndices(Info) == TArray<int32>({ 0, 1, 1, 1 }))
	CHECK_MESSAGE(TEXT("The corner reports length 1 and the wall run reports its full length."),
		SegmentLengths(Info) == TArray<int32>({ 1, 3, 3, 3 }))
}

N_TEST_HIGH(FNSortLine2DXYElementTests_ClassifyLine_CornerAtLastIndex,
	"NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElement::ClassifyLine::CornerAtLastIndex",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElementHarness;
	// A corner at the final index closes the leading wall run and produces a trailing length-1 segment.
	TArray<FNSortLinePointInfo> Info;
	FNSortLine2DXYElement::ClassifyLine({ 0.f, 0.f, 0.f, 1.f }, Tolerance, Info);

	CHECK_MESSAGE(TEXT("A turn at the last index should classify the final point as a corner."),
		Parts(Info) == TArray<int32>({ Wall, Wall, Wall, Left }))
	CHECK_MESSAGE(TEXT("The leading wall run is segment 0; the final corner is segment 1."),
		SegmentIndices(Info) == TArray<int32>({ 0, 0, 0, 1 }))
	CHECK_MESSAGE(TEXT("Wall run reports length 3, the trailing corner reports length 1."),
		SegmentLengths(Info) == TArray<int32>({ 3, 3, 3, 1 }))
	CHECK_MESSAGE(TEXT("Every point should own a non-empty segment."), AllSegmentsNonEmpty(Info))
}

N_TEST_HIGH(FNSortLine2DXYElementTests_ClassifyLine_ClosedSquare,
	"NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElement::ClassifyLine::ClosedSquare",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElementHarness;
	// A four-corner loop with no walls: every point is a corner owning its own length-1 segment, and the
	// segment indices must stay within the resolved-length array (the bounds invariant under all corners).
	TArray<FNSortLinePointInfo> Info;
	FNSortLine2DXYElement::ClassifyLine({ 1.f, 1.f, 1.f, 1.f }, Tolerance, Info);

	CHECK_MESSAGE(TEXT("A four-corner loop should classify every point as a left corner."),
		Parts(Info) == TArray<int32>({ Left, Left, Left, Left }))
	CHECK_MESSAGE(TEXT("Each corner owns a distinct, even-numbered segment index."),
		SegmentIndices(Info) == TArray<int32>({ 0, 2, 4, 6 }))
	CHECK_MESSAGE(TEXT("Every corner reports a segment length of 1."),
		SegmentLengths(Info) == TArray<int32>({ 1, 1, 1, 1 }))
	CHECK_MESSAGE(TEXT("Every point should own a non-empty segment."), AllSegmentsNonEmpty(Info))
}

N_TEST_HIGH(FNSortLine2DXYElementTests_ClassifyLine_SubsegmentIndexIsZeroBased,
	"NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElement::ClassifyLine::SubsegmentIndexIsZeroBased",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElementHarness;
	// Subsegment index is the 0-based position of a point within its segment: a corner is always 0, and
	// every wall run counts up from 0 -- including the leading run on an open line.
	TArray<FNSortLinePointInfo> StraightInfo;
	FNSortLine2DXYElement::ClassifyLine({ 0.f, 0.f, 0.f, 0.f }, Tolerance, StraightInfo);
	CHECK_MESSAGE(TEXT("The leading wall run should start its subsegment index at 0."),
		SubsegmentIndices(StraightInfo) == TArray<int32>({ 0, 1, 2, 3 }))

	TArray<FNSortLinePointInfo> CornerInfo;
	FNSortLine2DXYElement::ClassifyLine({ 0.f, 0.f, 1.f, 0.f, 0.f }, Tolerance, CornerInfo);
	CHECK_MESSAGE(TEXT("The corner is subsegment 0 and the following wall run restarts at 0."),
		SubsegmentIndices(CornerInfo) == TArray<int32>({ 0, 1, 0, 0, 1 }))
}

//
// ClassifyLine — WA-3: off-grid turns must still resolve to a named corner, and the deadzone separates
// walls from corners by sign rather than exact +-1 equality.
//

N_TEST_HIGH(FNSortLine2DXYElementTests_ClassifyLine_OffGridTurnsAreNamed,
	"NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElement::ClassifyLine::OffGridTurnsAreNamed",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElementHarness;
	// Regression for WA-3: a non-90 turn (e.g. ~45 degrees -> 0.707) used to enter the corner branch but
	// match neither +-1, leaving the point with the default classification. It must now resolve by sign.
	TArray<FNSortLinePointInfo> Info;
	FNSortLine2DXYElement::ClassifyLine({ 0.f, 0.707f, -0.707f }, Tolerance, Info);

	CHECK_MESSAGE(TEXT("An off-grid positive turn must classify as a left corner, not stay a wall."),
		Parts(Info) == TArray<int32>({ Wall, Left, Right }))
}

N_TEST_HIGH(FNSortLine2DXYElementTests_ClassifyLine_DeadzoneKeepsShallowTurnsAsWalls,
	"NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElement::ClassifyLine::DeadzoneKeepsShallowTurnsAsWalls",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElementHarness;
	// Float noise on a "straight" run and shallow sub-threshold turns stay walls instead of registering
	// as corners (the deadzone replaces the old exact == 0 test).
	TArray<FNSortLinePointInfo> Info;
	FNSortLine2DXYElement::ClassifyLine({ 0.f, 1.e-4f, 0.3f, -0.3f }, Tolerance, Info);

	CHECK_MESSAGE(TEXT("Turns within the deadzone should all classify as walls."),
		Parts(Info) == TArray<int32>({ Wall, Wall, Wall, Wall }))
	CHECK_MESSAGE(TEXT("With no corners the whole line is one segment of the full length."),
		SegmentLengths(Info) == TArray<int32>({ 4, 4, 4, 4 }))
}

N_TEST_MEDIUM(FNSortLine2DXYElementTests_ClassifyLine_SignMapsToLeftRight,
	"NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElement::ClassifyLine::SignMapsToLeftRight",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElementHarness;
	// Sign convention: a positive turn is a left corner, a negative turn is a right corner.
	TArray<FNSortLinePointInfo> LeftInfo;
	FNSortLine2DXYElement::ClassifyLine({ 0.f, 1.f }, Tolerance, LeftInfo);
	CHECK_MESSAGE(TEXT("A positive turn should classify as a left corner."),
		LeftInfo.Num() == 2 && LeftInfo[1].Part == ENSortLinePart::Left90)

	TArray<FNSortLinePointInfo> RightInfo;
	FNSortLine2DXYElement::ClassifyLine({ 0.f, -1.f }, Tolerance, RightInfo);
	CHECK_MESSAGE(TEXT("A negative turn should classify as a right corner."),
		RightInfo.Num() == 2 && RightInfo[1].Part == ENSortLinePart::Right90)
}

N_TEST_LOW(FNSortLine2DXYElementTests_ClassifyLine_Empty,
	"NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElement::ClassifyLine::Empty",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNSortLine2DXYElementHarness;
	// An empty input must produce empty output without touching the segment-length arrays.
	TArray<FNSortLinePointInfo> Info;
	FNSortLine2DXYElement::ClassifyLine({}, Tolerance, Info);
	CHECK_EQUALS("Classifying an empty line should produce no point info.", Info.Num(), 0)
}

#endif //WITH_TESTS

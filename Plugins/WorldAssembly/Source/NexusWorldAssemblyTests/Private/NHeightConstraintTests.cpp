// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Cell/NCellJunctionDetails.h"
#include "Collections/NWeightedIntegerArray.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNHeightConstraintHarness
{
	/** The socket size every "matching" junction and filter shares in these tests. */
	static FIntVector2 MatchingSocket() { return FIntVector2(2, 4); }

	/**
	 * A candidate cell with a 200-unit cube for bounds, centered on its pivot, carrying one junction of
	 * MatchingSocket sitting at the pivot. With an identity junction rotation the cell's world bounds are simply
	 * the cube translated to the attach point, which keeps the expected Z ranges below readable.
	 */
	static FNVirtualCellData MakeCell()
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-100.0), FVector(100.0));
		Cell.Junctions.Add(0, FNCellJunctionDetails());
		return Cell;
	}

	/** A MakeCell() candidate that may not be placed where its bounds would dip below Floor. */
	static FNVirtualCellData MakeFloorConstrainedCell(const double Floor)
	{
		FNVirtualCellData Cell = MakeCell();
		Cell.bUseMinimumFloor = true;
		Cell.MinimumFloor = Floor;
		return Cell;
	}

	/** A MakeCell() candidate that may not be placed where its bounds would reach above Ceiling. */
	static FNVirtualCellData MakeCeilingConstrainedCell(const double Ceiling)
	{
		FNVirtualCellData Cell = MakeCell();
		Cell.bUseMaximumCeiling = true;
		Cell.MaximumCeiling = Ceiling;
		return Cell;
	}

	/**
	 * Lift the cell's matching-rotation constraints, on the cell and on every one of its junctions.
	 *
	 * FNRotationConstraints defaults to enforcing a yaw-only interval, so a candidate that would have to pitch or
	 * roll into place is gated on rotation before the height gate is ever reached. A test that means to exercise
	 * the height gate against a pitched placement has to clear both vetoes or it passes on the wrong rejection.
	 */
	static void AllowAnyRotation(FNVirtualCellData& Cell)
	{
		Cell.CellDetails.RotationConstraints.bEnforceMatchingRotation = false;
		for (TPair<int32, FNCellJunctionDetails>& Junction : Cell.Junctions)
		{
			Junction.Value.RotationConstraints.bEnforceMatchingRotation = false;
		}
	}

	/**
	 * A filter attaching at AttachZ on the world axis, with an identity source facing.
	 * @remark bIsStartNode is left false, so the candidate resolves through the flipped junction-to-junction
	 *         placement math the graph expansion uses.
	 */
	static FNCellInputDataFilter MakeFilter(const double AttachZ)
	{
		FNCellInputDataFilter Filter;
		Filter.SocketSize = MatchingSocket();
		Filter.SourceQuat = FQuat::Identity;
		Filter.WorldPosition = FVector(0.0, 0.0, AttachZ);
		return Filter;
	}

	/** Run the filter and report whether the single candidate at index 0 survived every gate. */
	static bool IsCandidateSelectable(FNVirtualOrganContext& Context, const FNCellInputDataFilter& Filter)
	{
		FNWeightedIntegerArray CellIndices;
		TMap<int32, TArray<int32>> JunctionIndices;
		Context.FilterCellInputData(Filter, CellIndices, JunctionIndices);
		return CellIndices.HasValue(0);
	}
}

N_TEST_HIGH(FNHeightConstraintTests_IsGatedByHeight_DisabledNeverGates,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::IsGatedByHeight::DisabledNeverGates",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Both limits off is the "no constraint" state every unconfigured cell and organ is in, and it must never gate
	// regardless of where the candidate would land or what the (then meaningless) limit values hold.
	const FBox FarBelow(FVector(-100000.0), FVector(-99000.0));
	const FBox FarAbove(FVector(99000.0), FVector(100000.0));

	CHECK_FALSE_MESSAGE(TEXT("A disabled gate must not reject a candidate far below the floor value."),
		FNVirtualOrganContext::IsGatedByHeight(FarBelow, false, 0.0, false, 0.0))
	CHECK_FALSE_MESSAGE(TEXT("A disabled gate must not reject a candidate far above the ceiling value."),
		FNVirtualOrganContext::IsGatedByHeight(FarAbove, false, 0.0, false, 0.0))
}

N_TEST_HIGH(FNHeightConstraintTests_IsGatedByHeight_LimitsAreInclusive,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::IsGatedByHeight::LimitsAreInclusive",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Bounds resting exactly on the floor, or reaching exactly to the ceiling, sit inside the allowed window; only
	// a genuine overshoot gates. A cell authored to sit flush on a floor of 0 is the common case and must pass.
	const FBox RestingOnZero(FVector(-100.0, -100.0, 0.0), FVector(100.0, 100.0, 200.0));
	CHECK_FALSE_MESSAGE(TEXT("Bounds resting exactly on the floor must be allowed."),
		FNVirtualOrganContext::IsGatedByHeight(RestingOnZero, true, 0.0, false, 0.0))

	const FBox DippingBelowZero(FVector(-100.0, -100.0, -1.0), FVector(100.0, 100.0, 199.0));
	CHECK_MESSAGE(TEXT("Bounds a single unit below the floor must be gated."),
		FNVirtualOrganContext::IsGatedByHeight(DippingBelowZero, true, 0.0, false, 0.0))

	const FBox ReachingToZero(FVector(-100.0, -100.0, -200.0), FVector(100.0, 100.0, 0.0));
	CHECK_FALSE_MESSAGE(TEXT("Bounds reaching exactly to the ceiling must be allowed."),
		FNVirtualOrganContext::IsGatedByHeight(ReachingToZero, false, 0.0, true, 0.0))

	const FBox PokingAboveZero(FVector(-100.0, -100.0, -199.0), FVector(100.0, 100.0, 1.0));
	CHECK_MESSAGE(TEXT("Bounds a single unit above the ceiling must be gated."),
		FNVirtualOrganContext::IsGatedByHeight(PokingAboveZero, false, 0.0, true, 0.0))
}

N_TEST_MEDIUM(FNHeightConstraintTests_IsGatedByHeight_InvalidBoundsNeverGate,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::IsGatedByHeight::InvalidBoundsNeverGate",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A candidate carrying no bounds cannot be judged on height. Admitting it matches how the collision paths treat
	// an unbounded hull, and keeps a cell with unauthored bounds from silently vanishing from every tissue it is in.
	CHECK_FALSE_MESSAGE(TEXT("An invalid bounds must not be gated by the floor."),
		FNVirtualOrganContext::IsGatedByHeight(FBox(ForceInit), true, 0.0, false, 0.0))
	CHECK_FALSE_MESSAGE(TEXT("An invalid bounds must not be gated by the ceiling."),
		FNVirtualOrganContext::IsGatedByHeight(FBox(ForceInit), false, 0.0, true, 0.0))
}

N_TEST_CRITICAL(FNHeightConstraintTests_FilterCellInputData_MinimumFloorGatesOnBoundsNotPivot,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData::MinimumFloorGatesOnBoundsNotPivot",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The gate reads the candidate's world *bounds*, not its pivot. A 200-unit cube attaching at Z 50 sits with its
	// pivot comfortably above a floor of 0 while its underside reaches -50, and must be rejected for that underside.
	// Raising the attach point to Z 100 puts the underside flush on the floor, which is allowed.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNHeightConstraintHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("HeightTest"));
	Context.CellInputDataSummary.bAnyHeightConstraints = true;
	Context.CellInputData.Add(MakeFloorConstrainedCell(0.0));

	CHECK_FALSE_MESSAGE(TEXT("A cell whose bounds dip below the floor must be excluded, even with its pivot above it."),
		IsCandidateSelectable(Context, MakeFilter(50.0)))
	CHECK_MESSAGE(TEXT("A cell whose bounds rest flush on the floor must stay selectable."),
		IsCandidateSelectable(Context, MakeFilter(100.0)))
}

N_TEST_HIGH(FNHeightConstraintTests_FilterCellInputData_MaximumCeilingGatesOnBoundsNotPivot,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData::MaximumCeilingGatesOnBoundsNotPivot",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The ceiling mirrors the floor: a cube attaching at Z -50 keeps its pivot below a ceiling of 0 while its top
	// reaches +50, and is rejected. Dropping the attach point to Z -100 puts its top flush on the ceiling.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNHeightConstraintHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("HeightTest"));
	Context.CellInputDataSummary.bAnyHeightConstraints = true;
	Context.CellInputData.Add(MakeCeilingConstrainedCell(0.0));

	CHECK_FALSE_MESSAGE(TEXT("A cell whose bounds reach above the ceiling must be excluded, even with its pivot below it."),
		IsCandidateSelectable(Context, MakeFilter(-50.0)))
	CHECK_MESSAGE(TEXT("A cell whose bounds reach exactly to the ceiling must stay selectable."),
		IsCandidateSelectable(Context, MakeFilter(-100.0)))
}

N_TEST_HIGH(FNHeightConstraintTests_FilterCellInputData_InactiveGateAdmitsAnyHeight,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData::InactiveGateAdmitsAnyHeight",
	N_TEST_CONTEXT_ANYWHERE)
{
	// With neither the organ nor any cell in the pool declaring a limit, the gate is skipped entirely and placement
	// is unaffected at any height. This is the state every existing tissue and organ loads in.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNHeightConstraintHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("HeightTest"));
	Context.CellInputData.Add(MakeCell());

	CHECK_MESSAGE(TEXT("An unconstrained cell must be selectable far below world zero."),
		IsCandidateSelectable(Context, MakeFilter(-100000.0)))
	CHECK_MESSAGE(TEXT("An unconstrained cell must be selectable far above world zero."),
		IsCandidateSelectable(Context, MakeFilter(100000.0)))
}

N_TEST_HIGH(FNHeightConstraintTests_FilterCellInputData_OrganFloorAppliesToUnconstrainedCells,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData::OrganFloorAppliesToUnconstrainedCells",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The organ's floor is a blanket gate: a cell declaring nothing of its own is still held to it. This is the
	// "nothing below zero" case, and it is the only vertical limit an unbound organ can express.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNHeightConstraintHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("HeightTest"));
	Context.bUseMinimumFloor = true;
	Context.MinimumFloor = 0.0;
	Context.CellInputData.Add(MakeCell());

	CHECK_FALSE_MESSAGE(TEXT("An unconstrained cell dipping below the organ's floor must be excluded."),
		IsCandidateSelectable(Context, MakeFilter(50.0)))
	CHECK_MESSAGE(TEXT("An unconstrained cell clearing the organ's floor must stay selectable."),
		IsCandidateSelectable(Context, MakeFilter(100.0)))
}

N_TEST_CRITICAL(FNHeightConstraintTests_FilterCellInputData_EntryNarrowsOrganWindowButCannotWidenIt,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData::EntryNarrowsOrganWindowButCannotWidenIt",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Organ and entry limits combine by narrowing: the higher floor wins. Index 0 raises the floor above the
	// organ's and is held to its own stricter value; index 1 lowers it below the organ's and is still held to the
	// organ's, so an entry can never author its way underneath the organ it is placed in.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNHeightConstraintHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("HeightTest"));
	Context.CellInputDataSummary.bAnyHeightConstraints = true;
	Context.bUseMinimumFloor = true;
	Context.MinimumFloor = 0.0;
	Context.CellInputData.Add(MakeFloorConstrainedCell(500.0));

	FNWeightedIntegerArray CellIndices;
	TMap<int32, TArray<int32>> JunctionIndices;

	// Attaching at Z 200 puts the cube's underside at 100: clear of the organ's floor of 0, but under its own 500.
	Context.FilterCellInputData(MakeFilter(200.0), CellIndices, JunctionIndices);
	CHECK_FALSE_MESSAGE(TEXT("An entry floor above the organ's must be the one enforced."), CellIndices.HasValue(0))

	// Attaching at Z 600 puts the underside at 500, flush on the stricter of the two floors.
	Context.FilterCellInputData(MakeFilter(600.0), CellIndices, JunctionIndices);
	CHECK_MESSAGE(TEXT("A candidate clearing the stricter entry floor must stay selectable."), CellIndices.HasValue(0))

	// An entry floor below the organ's is the looser of the two and must not take effect. Attaching at Z 50 puts
	// the underside at -50: under the organ's floor of 0, though well clear of the entry's -1000.
	FNVirtualOrganContext PermissiveEntry(1234ull, TEXT("HeightTest"));
	PermissiveEntry.CellInputDataSummary.bAnyHeightConstraints = true;
	PermissiveEntry.bUseMinimumFloor = true;
	PermissiveEntry.MinimumFloor = 0.0;
	PermissiveEntry.CellInputData.Add(MakeFloorConstrainedCell(-1000.0));

	CHECK_FALSE_MESSAGE(TEXT("An entry floor below the organ's must not widen the organ's window."),
		IsCandidateSelectable(PermissiveEntry, MakeFilter(50.0)))
}

N_TEST_HIGH(FNHeightConstraintTests_FilterCellInputData_RotationIsBakedIntoTestedBounds,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData::RotationIsBakedIntoTestedBounds",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The gate tests the rotation-baked AABB, not the authored one. A cell that is wide and flat clears a floor
	// lying just under it while upright, but stood on end by the placement rotation its 400-unit half-width becomes
	// its vertical reach and it no longer does. Placed through the start-node path, where the cell simply takes the
	// source facing, so the rotation under test is stated directly rather than composed.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNHeightConstraintHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("HeightTest"));
	Context.CellInputDataSummary.bAnyHeightConstraints = true;

	// Wide in X, shallow in Z: +/-400 across, +/-50 tall. Rotation constraints are lifted so the pitched placement
	// is judged on height rather than being turned away by the default yaw-only matching interval first.
	FNVirtualCellData Cell = MakeFloorConstrainedCell(-100.0);
	Cell.CellDetails.Bounds = FBox(FVector(-400.0, -100.0, -50.0), FVector(400.0, 100.0, 50.0));
	AllowAnyRotation(Cell);
	Context.CellInputData.Add(Cell);

	// Upright at the origin the cell reaches down to -50, clearing a floor of -100.
	FNCellInputDataFilter Upright = MakeFilter(0.0);
	Upright.bIsStartNode = true;
	CHECK_MESSAGE(TEXT("Upright, the cell clears the floor and must stay selectable."),
		IsCandidateSelectable(Context, Upright))

	// A 90-degree pitch swings X onto Z, so the same cell now reaches down to -400 and breaks the same floor.
	FNCellInputDataFilter Pitched = MakeFilter(0.0);
	Pitched.bIsStartNode = true;
	Pitched.SourceQuat = FQuat(FRotator(90.0, 0.0, 0.0));
	CHECK_FALSE_MESSAGE(TEXT("Stood on end, the same cell breaks the floor and must be excluded."),
		IsCandidateSelectable(Context, Pitched))
}

N_TEST_CRITICAL(FNHeightConstraintTests_FilterCellInputData_StartNodeResolvesUnflippedPlacement,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData::StartNodeResolvesUnflippedPlacement",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A start cell mates to a bone by *matching* its facing (FNOrganGraphBuilderTask::StartGraph), where every other
	// placement opposes the source junction's — so the two paths resolve a candidate to different places, and the
	// height gate has to compose each the way its own placement path will.
	//
	// The setup makes that divergence visible: a junction offset 100 along the cell's X, attaching at the world
	// origin with the source pitched 90 degrees nose-up. Matching that facing swings the offset to +Z, dropping the
	// cell's pivot to Z -100 and its bounds to [-200, 0]. Opposing it swings the offset to -Z, lifting the pivot to
	// Z +100 and its bounds to [0, 200]. Against a floor of -100 the two verdicts are opposites, so reusing the
	// wrong composition for a start cell cannot pass this quietly. The floor sits 100 clear of either bound rather
	// than flush against one, since a pitched rotation matrix carries cos(90) as an epsilon rather than a true zero.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNHeightConstraintHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("HeightTest"));
	Context.CellInputDataSummary.bAnyHeightConstraints = true;

	// Rotation constraints are lifted so both placements are judged on height rather than turned away by the
	// default yaw-only matching interval, which a 90-degree pitch fails on either path.
	FNVirtualCellData Cell = MakeFloorConstrainedCell(-100.0);
	Cell.Junctions[0].WorldLocation = FVector(100.0, 0.0, 0.0);
	AllowAnyRotation(Cell);
	Context.CellInputData.Add(Cell);

	FNCellInputDataFilter StartFilter = MakeFilter(0.0);
	StartFilter.SourceQuat = FQuat(FRotator(90.0, 0.0, 0.0));
	StartFilter.bIsStartNode = true;
	CHECK_FALSE_MESSAGE(TEXT("Matching the bone facing drops the start cell below the floor; it must be excluded."),
		IsCandidateSelectable(Context, StartFilter))

	FNCellInputDataFilter ExpansionFilter = MakeFilter(0.0);
	ExpansionFilter.SourceQuat = FQuat(FRotator(90.0, 0.0, 0.0));
	CHECK_MESSAGE(TEXT("Opposing the source junction lifts the same cell clear of the floor; it must stay selectable."),
		IsCandidateSelectable(Context, ExpansionFilter))
}

#endif //WITH_TESTS

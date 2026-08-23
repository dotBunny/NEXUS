// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Macros/NTestMacros.h"
#include "PCG/Elements/NAdjustRowsElement.h"

namespace NEXUS::UnitTests::NCore::FNAdjustRowsElementHarness
{
	/** Groups the supplied positions and renders the resulting rows, keeping each test's call site to one line. */
	static FString AssignAndDescribe(const TArray<double>& Positions, const double Tolerance)
	{
		TArray<int32> RowIndices;
		FNAdjustRowsElement::AssignRowIndices(Positions, Tolerance, RowIndices);
		return FString::JoinBy(RowIndices, TEXT(","), [](const int32 RowIndex) { return FString::FromInt(RowIndex); });
	}
}

using namespace NEXUS::UnitTests::NCore::FNAdjustRowsElementHarness;

N_TEST_HIGH(FNAdjustRowsElementTests_GetAxisIndex_MapsEachAxisToItsComponent,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::GetAxisIndex::MapsEachAxisToItsComponent",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The element addresses the translation through these indices, so a wrong mapping would silently
	// group by one axis and offset along another.
	CHECK_EQUALS("X should map to component 0.", FNAdjustRowsElement::GetAxisIndex(ENAxis::X), 0);
	CHECK_EQUALS("Y should map to component 1.", FNAdjustRowsElement::GetAxisIndex(ENAxis::Y), 1);
	CHECK_EQUALS("Z should map to component 2.", FNAdjustRowsElement::GetAxisIndex(ENAxis::Z), 2);
}

N_TEST_MEDIUM(FNAdjustRowsElementTests_GetAxisIndex_NoneHasNoComponent,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::GetAxisIndex::NoneHasNoComponent",
	N_TEST_CONTEXT_ANYWHERE)
{
	// ENAxis carries a None, and it must not fall through to component zero and quietly act as X.
	CHECK_EQUALS("None should report no component.", FNAdjustRowsElement::GetAxisIndex(ENAxis::None), static_cast<int32>(INDEX_NONE));
}

N_TEST_HIGH(FNAdjustRowsElementTests_AssignRowIndices_GroupsMatchingPositionsIntoOneRow,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::AssignRowIndices::GroupsMatchingPositionsIntoOneRow",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The common case: a regular grid, three points per row, rows numbered along the row axis.
	const FString RowIndices = AssignAndDescribe({0.0, 0.0, 0.0, 100.0, 100.0, 100.0, 200.0, 200.0, 200.0}, 1.0);
	CHECK_EQUALS("Each distinct position should form its own row.", *RowIndices, TEXT("0,0,0,1,1,1,2,2,2"));
}

N_TEST_HIGH(FNAdjustRowsElementTests_AssignRowIndices_PreservesInputOrder,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::AssignRowIndices::PreservesInputOrder",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Row numbering is derived from sorted positions, but each index must come back against its original
	// slot — the element leans on this to leave the output point order alone.
	const FString RowIndices = AssignAndDescribe({200.0, 0.0, 100.0, 0.0, 200.0, 100.0}, 1.0);
	CHECK_EQUALS("Row indices should be returned in input order, not sorted order.", *RowIndices, TEXT("2,0,1,0,2,1"));
}

N_TEST_HIGH(FNAdjustRowsElementTests_AssignRowIndices_ToleranceAbsorbsJitter,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::AssignRowIndices::ToleranceAbsorbsJitter",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Points nudged off a perfect grid should still land in the row they visually belong to.
	const FString RowIndices = AssignAndDescribe({0.0, 0.5, 100.0, 100.4}, 1.0);
	CHECK_EQUALS("Positions within the tolerance should share a row.", *RowIndices, TEXT("0,0,1,1"));
}

N_TEST_HIGH(FNAdjustRowsElementTests_AssignRowIndices_AnchorsOnRowStart,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::AssignRowIndices::AnchorsOnRowStart",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Every step here is inside the tolerance, so measuring against the previous point would swallow all
	// four into one row. Anchoring on the position that opened the row caps a row's spread at the tolerance.
	const FString RowIndices = AssignAndDescribe({0.0, 0.9, 1.8, 2.7}, 1.0);
	CHECK_EQUALS("A run of close steps should still break into rows.", *RowIndices, TEXT("0,0,1,1"));
}

N_TEST_MEDIUM(FNAdjustRowsElementTests_AssignRowIndices_ZeroToleranceGroupsExactMatchesOnly,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::AssignRowIndices::ZeroToleranceGroupsExactMatchesOnly",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FString RowIndices = AssignAndDescribe({0.0, 0.0, 0.5, 0.5}, 0.0);
	CHECK_EQUALS("A zero tolerance should only group identical positions.", *RowIndices, TEXT("0,0,1,1"));
}

N_TEST_MEDIUM(FNAdjustRowsElementTests_AssignRowIndices_NumbersRowsFromTheNegativeSide,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::AssignRowIndices::NumbersRowsFromTheNegativeSide",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Row zero is always the most negative row, wherever the cloud sits relative to the origin.
	const FString RowIndices = AssignAndDescribe({100.0, -100.0, 0.0}, 1.0);
	CHECK_EQUALS("The most negative row should be row zero.", *RowIndices, TEXT("2,0,1"));
}

N_TEST_MEDIUM(FNAdjustRowsElementTests_AssignRowIndices_EmptyInputProducesNoRows,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::AssignRowIndices::EmptyInputProducesNoRows",
	N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> RowIndices;
	FNAdjustRowsElement::AssignRowIndices(TArray<double>(), 1.0, RowIndices);
	CHECK_EQUALS("An empty point set should produce no row indices.", RowIndices.Num(), 0);
}

N_TEST_HIGH(FNAdjustRowsElementTests_GetFixedRowIndex_BucketsPositivePositions,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::GetFixedRowIndex::BucketsPositivePositions",
	N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_EQUALS("250 in 100-wide bands should land in row 2.", FNAdjustRowsElement::GetFixedRowIndex(250.0, 100.0), 2);
	CHECK_EQUALS("A band's lower edge should belong to that band.", FNAdjustRowsElement::GetFixedRowIndex(200.0, 100.0), 2);
}

N_TEST_HIGH(FNAdjustRowsElementTests_GetFixedRowIndex_FloorsNegativePositionsAwayFromZero,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::GetFixedRowIndex::FloorsNegativePositionsAwayFromZero",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Truncation toward zero would put both of these in row 0 and break the alternating pattern at the origin.
	CHECK_EQUALS("-50 should floor to row -1.", FNAdjustRowsElement::GetFixedRowIndex(-50.0, 100.0), -1);
	CHECK_EQUALS("-150 should floor to row -2.", FNAdjustRowsElement::GetFixedRowIndex(-150.0, 100.0), -2);
}

N_TEST_MEDIUM(FNAdjustRowsElementTests_GetFixedRowIndex_NonPositiveRowSizeCollapsesToFirstRow,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::GetFixedRowIndex::NonPositiveRowSizeCollapsesToFirstRow",
	N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_EQUALS("A zero row size should collapse everything into row 0.", FNAdjustRowsElement::GetFixedRowIndex(500.0, 0.0), 0);
}

N_TEST_HIGH(FNAdjustRowsElementTests_ShouldOffsetRow_EvenParityMovesTheFirstRow,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::ShouldOffsetRow::EvenParityMovesTheFirstRow",
	N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("Row 0 should be offset under even parity."), FNAdjustRowsElement::ShouldOffsetRow(0, ENRowParity::Even));
	CHECK_FALSE_MESSAGE(TEXT("Row 1 should be left alone under even parity."), FNAdjustRowsElement::ShouldOffsetRow(1, ENRowParity::Even));
	CHECK_MESSAGE(TEXT("Row 2 should be offset under even parity."), FNAdjustRowsElement::ShouldOffsetRow(2, ENRowParity::Even));
}

N_TEST_HIGH(FNAdjustRowsElementTests_ShouldOffsetRow_OddParityLeavesTheFirstRow,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::ShouldOffsetRow::OddParityLeavesTheFirstRow",
	N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_FALSE_MESSAGE(TEXT("Row 0 should be left alone under odd parity."), FNAdjustRowsElement::ShouldOffsetRow(0, ENRowParity::Odd));
	CHECK_MESSAGE(TEXT("Row 1 should be offset under odd parity."), FNAdjustRowsElement::ShouldOffsetRow(1, ENRowParity::Odd));
}

N_TEST_MEDIUM(FNAdjustRowsElementTests_ShouldOffsetRow_NegativeRowsKeepAlternating,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::ShouldOffsetRow::NegativeRowsKeepAlternating",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Fixed-size detection hands out negative rows below the origin; a raw '%' would report -1 for row -1,
	// matching neither parity and stranding those rows unshifted.
	CHECK_FALSE_MESSAGE(TEXT("Row -1 should be left alone under even parity."), FNAdjustRowsElement::ShouldOffsetRow(-1, ENRowParity::Even));
	CHECK_MESSAGE(TEXT("Row -1 should be offset under odd parity."), FNAdjustRowsElement::ShouldOffsetRow(-1, ENRowParity::Odd));
	CHECK_MESSAGE(TEXT("Row -2 should be offset under even parity."), FNAdjustRowsElement::ShouldOffsetRow(-2, ENRowParity::Even));
}

N_TEST_HIGH(FNAdjustRowsElementTests_GetRowRotation_HonorsTheSelectedAxis,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::GetRowRotation::HonorsTheSelectedAxis",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The axis the rows turn around is its own setting, separate from the row and offset axes, so each of
	// the three has to actually turn around the one it names.
	const FQuat AroundX = FNAdjustRowsElement::GetRowRotation(90.0, ENAxis::X);
	CHECK_EQUALS("An X turn should leave forward alone.", AroundX.RotateVector(FVector::ForwardVector), FVector::ForwardVector);

	const FQuat AroundY = FNAdjustRowsElement::GetRowRotation(90.0, ENAxis::Y);
	CHECK_EQUALS("A Y turn should leave right alone.", AroundY.RotateVector(FVector::RightVector), FVector::RightVector);

	const FQuat AroundZ = FNAdjustRowsElement::GetRowRotation(90.0, ENAxis::Z);
	CHECK_EQUALS("A Z turn should leave up alone.", AroundZ.RotateVector(FVector::UpVector), FVector::UpVector);
}

N_TEST_HIGH(FNAdjustRowsElementTests_GetRowRotation_HalfTurnFlipsAlternateRows,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::GetRowRotation::HalfTurnFlipsAlternateRows",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The reason the setting exists: half a turn on alternate rows interlocks the meshes instead of
	// repeating them, which is the whole difference between a run of rock and a run of copies.
	const FQuat HalfTurn = FNAdjustRowsElement::GetRowRotation(180.0, ENAxis::Z);
	CHECK_MESSAGE(TEXT("A half turn should reverse forward."),
		HalfTurn.RotateVector(FVector::ForwardVector).Equals(-FVector::ForwardVector, UE_KINDA_SMALL_NUMBER));
}

N_TEST_MEDIUM(FNAdjustRowsElementTests_GetRowRotation_NoAxisLeavesTheRowUnrotated,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::GetRowRotation::NoAxisLeavesTheRowUnrotated",
	N_TEST_CONTEXT_ANYWHERE)
{
	// ENAxis brings a None along; it must not fall through to a real axis and turn the row anyway, which
	// is exactly what a switch default would have done.
	const FQuat Rotation = FNAdjustRowsElement::GetRowRotation(90.0, ENAxis::None);
	CHECK_EQUALS("No axis should leave forward alone.", Rotation.RotateVector(FVector::ForwardVector), FVector::ForwardVector);
	CHECK_EQUALS("No axis should leave right alone.", Rotation.RotateVector(FVector::RightVector), FVector::RightVector);
	CHECK_EQUALS("No axis should leave up alone.", Rotation.RotateVector(FVector::UpVector), FVector::UpVector);
}

N_TEST_MEDIUM(FNAdjustRowsElementTests_GetRowRotation_ZeroDegreesLeavesTheRowUnrotated,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::GetRowRotation::ZeroDegreesLeavesTheRowUnrotated",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The default, and what the node checks against to decide whether the rotation pass runs at all.
	CHECK_MESSAGE(TEXT("A zero turn should be the identity."),
		FNAdjustRowsElement::GetRowRotation(0.0, ENAxis::Z).IsIdentity(UE_KINDA_SMALL_NUMBER));
}

N_TEST_HIGH(FNAdjustRowsElementTests_GetRowRotation_ParityPicksTheOppositeRowsFromTheOffset,
	"NEXUS::UnitTests::NCore::FNAdjustRowsElement::GetRowRotation::ParityPicksTheOppositeRowsFromTheOffset",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The rotation carries its own parity so a layout can shift one set of rows and turn the other. Both
	// adjustments share ShouldOffsetRow, so what this pins is that opposite parities really do select
	// disjoint sets of rows — otherwise holding them apart would have bought nothing.
	for (int32 RowIndex = -4; RowIndex <= 4; ++RowIndex)
	{
		const bool bEven = FNAdjustRowsElement::ShouldOffsetRow(RowIndex, ENRowParity::Even);
		const bool bOdd = FNAdjustRowsElement::ShouldOffsetRow(RowIndex, ENRowParity::Odd);
		CHECK_MESSAGE(TEXT("A row should match exactly one of the two parities."), bEven != bOdd);
	}
}

#endif //WITH_TESTS

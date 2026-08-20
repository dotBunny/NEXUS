// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Macros/NTestMacros.h"
#include "PCG/Elements/NStaggerRowsElement.h"

namespace NEXUS::UnitTests::NCore::FNStaggerRowsElementHarness
{
	/** Groups the supplied positions and renders the resulting rows, keeping each test's call site to one line. */
	static FString AssignAndDescribe(const TArray<double>& Positions, const double Tolerance)
	{
		TArray<int32> RowIndices;
		FNStaggerRowsElement::AssignRowIndices(Positions, Tolerance, RowIndices);
		return FString::JoinBy(RowIndices, TEXT(","), [](const int32 RowIndex) { return FString::FromInt(RowIndex); });
	}
}

using namespace NEXUS::UnitTests::NCore::FNStaggerRowsElementHarness;

N_TEST_HIGH(FNStaggerRowsElementTests_GetAxisIndex_MapsEachAxisToItsComponent,
	"NEXUS::UnitTests::NCore::FNStaggerRowsElement::GetAxisIndex::MapsEachAxisToItsComponent",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The element addresses the translation through these indices, so a wrong mapping would silently
	// group by one axis and offset along another.
	CHECK_EQUALS("X should map to component 0.", FNStaggerRowsElement::GetAxisIndex(ENAxis::X), 0);
	CHECK_EQUALS("Y should map to component 1.", FNStaggerRowsElement::GetAxisIndex(ENAxis::Y), 1);
	CHECK_EQUALS("Z should map to component 2.", FNStaggerRowsElement::GetAxisIndex(ENAxis::Z), 2);
}

N_TEST_MEDIUM(FNStaggerRowsElementTests_GetAxisIndex_NoneHasNoComponent,
	"NEXUS::UnitTests::NCore::FNStaggerRowsElement::GetAxisIndex::NoneHasNoComponent",
	N_TEST_CONTEXT_ANYWHERE)
{
	// ENAxis carries a None, and it must not fall through to component zero and quietly act as X.
	CHECK_EQUALS("None should report no component.", FNStaggerRowsElement::GetAxisIndex(ENAxis::None), static_cast<int32>(INDEX_NONE));
}

N_TEST_HIGH(FNStaggerRowsElementTests_AssignRowIndices_GroupsMatchingPositionsIntoOneRow,
	"NEXUS::UnitTests::NCore::FNStaggerRowsElement::AssignRowIndices::GroupsMatchingPositionsIntoOneRow",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The common case: a regular grid, three points per row, rows numbered along the row axis.
	const FString RowIndices = AssignAndDescribe({0.0, 0.0, 0.0, 100.0, 100.0, 100.0, 200.0, 200.0, 200.0}, 1.0);
	CHECK_EQUALS("Each distinct position should form its own row.", *RowIndices, TEXT("0,0,0,1,1,1,2,2,2"));
}

N_TEST_HIGH(FNStaggerRowsElementTests_AssignRowIndices_PreservesInputOrder,
	"NEXUS::UnitTests::NCore::FNStaggerRowsElement::AssignRowIndices::PreservesInputOrder",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Row numbering is derived from sorted positions, but each index must come back against its original
	// slot — the element leans on this to leave the output point order alone.
	const FString RowIndices = AssignAndDescribe({200.0, 0.0, 100.0, 0.0, 200.0, 100.0}, 1.0);
	CHECK_EQUALS("Row indices should be returned in input order, not sorted order.", *RowIndices, TEXT("2,0,1,0,2,1"));
}

N_TEST_HIGH(FNStaggerRowsElementTests_AssignRowIndices_ToleranceAbsorbsJitter,
	"NEXUS::UnitTests::NCore::FNStaggerRowsElement::AssignRowIndices::ToleranceAbsorbsJitter",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Points nudged off a perfect grid should still land in the row they visually belong to.
	const FString RowIndices = AssignAndDescribe({0.0, 0.5, 100.0, 100.4}, 1.0);
	CHECK_EQUALS("Positions within the tolerance should share a row.", *RowIndices, TEXT("0,0,1,1"));
}

N_TEST_HIGH(FNStaggerRowsElementTests_AssignRowIndices_AnchorsOnRowStart,
	"NEXUS::UnitTests::NCore::FNStaggerRowsElement::AssignRowIndices::AnchorsOnRowStart",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Every step here is inside the tolerance, so measuring against the previous point would swallow all
	// four into one row. Anchoring on the position that opened the row caps a row's spread at the tolerance.
	const FString RowIndices = AssignAndDescribe({0.0, 0.9, 1.8, 2.7}, 1.0);
	CHECK_EQUALS("A run of close steps should still break into rows.", *RowIndices, TEXT("0,0,1,1"));
}

N_TEST_MEDIUM(FNStaggerRowsElementTests_AssignRowIndices_ZeroToleranceGroupsExactMatchesOnly,
	"NEXUS::UnitTests::NCore::FNStaggerRowsElement::AssignRowIndices::ZeroToleranceGroupsExactMatchesOnly",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FString RowIndices = AssignAndDescribe({0.0, 0.0, 0.5, 0.5}, 0.0);
	CHECK_EQUALS("A zero tolerance should only group identical positions.", *RowIndices, TEXT("0,0,1,1"));
}

N_TEST_MEDIUM(FNStaggerRowsElementTests_AssignRowIndices_NumbersRowsFromTheNegativeSide,
	"NEXUS::UnitTests::NCore::FNStaggerRowsElement::AssignRowIndices::NumbersRowsFromTheNegativeSide",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Row zero is always the most negative row, wherever the cloud sits relative to the origin.
	const FString RowIndices = AssignAndDescribe({100.0, -100.0, 0.0}, 1.0);
	CHECK_EQUALS("The most negative row should be row zero.", *RowIndices, TEXT("2,0,1"));
}

N_TEST_MEDIUM(FNStaggerRowsElementTests_AssignRowIndices_EmptyInputProducesNoRows,
	"NEXUS::UnitTests::NCore::FNStaggerRowsElement::AssignRowIndices::EmptyInputProducesNoRows",
	N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> RowIndices;
	FNStaggerRowsElement::AssignRowIndices(TArray<double>(), 1.0, RowIndices);
	CHECK_EQUALS("An empty point set should produce no row indices.", RowIndices.Num(), 0);
}

N_TEST_HIGH(FNStaggerRowsElementTests_GetFixedRowIndex_BucketsPositivePositions,
	"NEXUS::UnitTests::NCore::FNStaggerRowsElement::GetFixedRowIndex::BucketsPositivePositions",
	N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_EQUALS("250 in 100-wide bands should land in row 2.", FNStaggerRowsElement::GetFixedRowIndex(250.0, 100.0), 2);
	CHECK_EQUALS("A band's lower edge should belong to that band.", FNStaggerRowsElement::GetFixedRowIndex(200.0, 100.0), 2);
}

N_TEST_HIGH(FNStaggerRowsElementTests_GetFixedRowIndex_FloorsNegativePositionsAwayFromZero,
	"NEXUS::UnitTests::NCore::FNStaggerRowsElement::GetFixedRowIndex::FloorsNegativePositionsAwayFromZero",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Truncation toward zero would put both of these in row 0 and break the alternating pattern at the origin.
	CHECK_EQUALS("-50 should floor to row -1.", FNStaggerRowsElement::GetFixedRowIndex(-50.0, 100.0), -1);
	CHECK_EQUALS("-150 should floor to row -2.", FNStaggerRowsElement::GetFixedRowIndex(-150.0, 100.0), -2);
}

N_TEST_MEDIUM(FNStaggerRowsElementTests_GetFixedRowIndex_NonPositiveRowSizeCollapsesToFirstRow,
	"NEXUS::UnitTests::NCore::FNStaggerRowsElement::GetFixedRowIndex::NonPositiveRowSizeCollapsesToFirstRow",
	N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_EQUALS("A zero row size should collapse everything into row 0.", FNStaggerRowsElement::GetFixedRowIndex(500.0, 0.0), 0);
}

N_TEST_HIGH(FNStaggerRowsElementTests_ShouldOffsetRow_EvenParityMovesTheFirstRow,
	"NEXUS::UnitTests::NCore::FNStaggerRowsElement::ShouldOffsetRow::EvenParityMovesTheFirstRow",
	N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("Row 0 should be offset under even parity."), FNStaggerRowsElement::ShouldOffsetRow(0, ENRowParity::Even));
	CHECK_FALSE_MESSAGE(TEXT("Row 1 should be left alone under even parity."), FNStaggerRowsElement::ShouldOffsetRow(1, ENRowParity::Even));
	CHECK_MESSAGE(TEXT("Row 2 should be offset under even parity."), FNStaggerRowsElement::ShouldOffsetRow(2, ENRowParity::Even));
}

N_TEST_HIGH(FNStaggerRowsElementTests_ShouldOffsetRow_OddParityLeavesTheFirstRow,
	"NEXUS::UnitTests::NCore::FNStaggerRowsElement::ShouldOffsetRow::OddParityLeavesTheFirstRow",
	N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_FALSE_MESSAGE(TEXT("Row 0 should be left alone under odd parity."), FNStaggerRowsElement::ShouldOffsetRow(0, ENRowParity::Odd));
	CHECK_MESSAGE(TEXT("Row 1 should be offset under odd parity."), FNStaggerRowsElement::ShouldOffsetRow(1, ENRowParity::Odd));
}

N_TEST_MEDIUM(FNStaggerRowsElementTests_ShouldOffsetRow_NegativeRowsKeepAlternating,
	"NEXUS::UnitTests::NCore::FNStaggerRowsElement::ShouldOffsetRow::NegativeRowsKeepAlternating",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Fixed-size detection hands out negative rows below the origin; a raw '%' would report -1 for row -1,
	// matching neither parity and stranding those rows unshifted.
	CHECK_FALSE_MESSAGE(TEXT("Row -1 should be left alone under even parity."), FNStaggerRowsElement::ShouldOffsetRow(-1, ENRowParity::Even));
	CHECK_MESSAGE(TEXT("Row -1 should be offset under odd parity."), FNStaggerRowsElement::ShouldOffsetRow(-1, ENRowParity::Odd));
	CHECK_MESSAGE(TEXT("Row -2 should be offset under even parity."), FNStaggerRowsElement::ShouldOffsetRow(-2, ENRowParity::Even));
}

#endif //WITH_TESTS

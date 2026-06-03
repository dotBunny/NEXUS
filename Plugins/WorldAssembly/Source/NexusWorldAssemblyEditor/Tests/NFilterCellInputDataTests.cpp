// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Cell/NCellJunctionDetails.h"
#include "Collections/NWeightedIntegerArray.h"
#include "GameplayTagContainer.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNFilterCellInputDataHarness
{
	// The built-in tag symbols are declared without an export macro, so they can't be linked from the editor
	// module; request them by their registered name instead (matches the strings in NWorldAssemblyGameplayTags.cpp).
	static FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	/** The socket size every "matching" junction and filter shares in these tests. */
	static FIntVector2 MatchingSocket() { return FIntVector2(2, 4); }

	/** A junction of the given socket size, with default (permissive) rotation constraints. */
	static FNCellJunctionDetails Junction(const FIntVector2 SocketSize)
	{
		FNCellJunctionDetails Details;
		Details.SocketSize = SocketSize;
		return Details;
	}

	/** A candidate cell carrying a single junction of MatchingSocket and the supplied selection weighting. */
	static FNVirtualCellData MakeCell(const int32 Weighting = 1)
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-100.0), FVector(100.0));
		Cell.Weighting = Weighting;
		Cell.Junctions.Add(0, Junction(MatchingSocket()));
		return Cell;
	}

	/** A filter targeting MatchingSocket with an identity source facing (required rotation lands at a clean yaw). */
	static FNCellInputDataFilter MakeFilter()
	{
		FNCellInputDataFilter Filter;
		Filter.SocketSize = MatchingSocket();
		Filter.SourceQuat = FQuat::Identity;
		return Filter;
	}
}

N_TEST_CRITICAL(FNFilterCellInputDataTests_FilterCellInputData_SelectsMatchingCellAndJunction,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData::SelectsMatchingCellAndJunction",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The happy path through the whole gate pipeline: an unconstrained cell whose junction matches the filter
	// socket must end up in the weighted candidate list, with its matching junction key reported.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFilterCellInputDataHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("FilterTest"));
	Context.CellInputData.Add(MakeCell());

	FNWeightedIntegerArray CellIndices;
	TMap<int32, TArray<int32>> JunctionIndices;
	Context.FilterCellInputData(MakeFilter(), CellIndices, JunctionIndices);

	CHECK_MESSAGE(TEXT("The matching cell (index 0) should be selectable."), CellIndices.HasValue(0))
	if (!JunctionIndices.Contains(0))
	{
		ADD_ERROR("Expected junction indices to be reported for the matching cell.");
		return;
	}
	CHECK_MESSAGE(TEXT("The matching junction key 0 should be reported for the cell."), JunctionIndices[0].Contains(0))
}

N_TEST_HIGH(FNFilterCellInputDataTests_FilterCellInputData_WeightingProducesWeightedEntries,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData::WeightingProducesWeightedEntries",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A cell's Weighting must be honored as repeated entries in the weighted array so heavier cells are more
	// likely to be picked downstream.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFilterCellInputDataHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("FilterTest"));
	Context.CellInputData.Add(MakeCell(3));

	FNWeightedIntegerArray CellIndices;
	TMap<int32, TArray<int32>> JunctionIndices;
	Context.FilterCellInputData(MakeFilter(), CellIndices, JunctionIndices);

	CHECK_EQUALS("A cell with weighting 3 should contribute three weighted entries.", CellIndices.WeightedCount(), 3)
}

N_TEST_HIGH(FNFilterCellInputDataTests_FilterCellInputData_SocketSizeMismatchExcludesCell,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData::SocketSizeMismatchExcludesCell",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A cell whose only junction is a different socket size than the filter requires must be excluded entirely.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFilterCellInputDataHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("FilterTest"));
	Context.CellInputData.Add(MakeCell());

	FNCellInputDataFilter Filter = MakeFilter();
	Filter.SocketSize = FIntVector2(3, 3); // No junction provides this size.

	FNWeightedIntegerArray CellIndices;
	TMap<int32, TArray<int32>> JunctionIndices;
	Context.FilterCellInputData(Filter, CellIndices, JunctionIndices);

	CHECK_FALSE_MESSAGE(TEXT("A socket-size mismatch must leave the candidate list empty."), CellIndices.HasData())
	CHECK_FALSE_MESSAGE(TEXT("No junctions should be reported for an excluded cell."), JunctionIndices.Contains(0))
}

N_TEST_HIGH(FNFilterCellInputDataTests_FilterCellInputData_OnlyMatchingSizeJunctionsReported,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData::OnlyMatchingSizeJunctionsReported",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A cell can carry junctions of several sizes; only those matching the filter socket should be reported,
	// so the selection downstream never tries to mate against a wrong-sized junction.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFilterCellInputDataHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("FilterTest"));
	FNVirtualCellData Cell = MakeCell();          // key 0 is MatchingSocket
	Cell.Junctions.Add(1, Junction(FIntVector2(3, 3))); // key 1 is a non-matching size
	Context.CellInputData.Add(Cell);

	FNWeightedIntegerArray CellIndices;
	TMap<int32, TArray<int32>> JunctionIndices;
	Context.FilterCellInputData(MakeFilter(), CellIndices, JunctionIndices);

	if (!JunctionIndices.Contains(0))
	{
		ADD_ERROR("Expected the cell to be selectable on its matching junction.");
		return;
	}
	CHECK_MESSAGE(TEXT("The matching-size junction key 0 should be reported."), JunctionIndices[0].Contains(0))
	CHECK_FALSE_MESSAGE(TEXT("The non-matching-size junction key 1 should be omitted."), JunctionIndices[0].Contains(1))
}

N_TEST_HIGH(FNFilterCellInputDataTests_FilterCellInputData_RequiredContextTagGates,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData::RequiredContextTagGates",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The context-tags gate (otherwise only reachable through the full filter): a cell that requires a context
	// tag is excluded while that tag is absent, and becomes selectable once the context provides it.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFilterCellInputDataHarness;

	const FGameplayTag Required = Tag(TEXT("NEXUS.WorldAssembly.Behavior.Starter"));

	FNVirtualOrganContext Context(1234ull, TEXT("FilterTest"));
	FNVirtualCellData Cell = MakeCell();
	Cell.ContextTagsRequired = FGameplayTagContainer(Required);
	Context.CellInputData.Add(Cell);

	FNWeightedIntegerArray BeforeIndices;
	TMap<int32, TArray<int32>> BeforeJunctions;
	Context.FilterCellInputData(MakeFilter(), BeforeIndices, BeforeJunctions);
	CHECK_FALSE_MESSAGE(TEXT("A cell requiring an absent context tag must be excluded."), BeforeIndices.HasData())

	// Provide the required context tag and re-filter.
	Context.ContextTags = FGameplayTagContainer(Required);

	FNWeightedIntegerArray AfterIndices;
	TMap<int32, TArray<int32>> AfterJunctions;
	Context.FilterCellInputData(MakeFilter(), AfterIndices, AfterJunctions);
	CHECK_MESSAGE(TEXT("Once the context provides the required tag the cell must be selectable."), AfterIndices.HasValue(0))
}

N_TEST_HIGH(FNFilterCellInputDataTests_FilterCellInputData_PlacedUniqueTagGates,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData::PlacedUniqueTagGates",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The unique-tags gate (otherwise only reachable through the full filter): once a unique tag has been placed,
	// a candidate carrying that tag is excluded, while a candidate without it remains selectable.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFilterCellInputDataHarness;

	const FGameplayTag UniqueTag = Tag(TEXT("NEXUS.WorldAssembly.Behavior.Starter"));

	FNVirtualOrganContext Context(1234ull, TEXT("FilterTest"));

	FNVirtualCellData Tagged = MakeCell();   // index 0 — carries the unique tag
	Tagged.AssemblyTags = FGameplayTagContainer(UniqueTag);
	FNVirtualCellData Untagged = MakeCell();  // index 1 — carries nothing
	Context.CellInputData.Add(Tagged);
	Context.CellInputData.Add(Untagged);

	// Mark the tag as already placed (and therefore exhausted) in the graph so far.
	Context.PlacedTagGroups.AppendUniqueTags(FGameplayTagContainer(UniqueTag));

	FNWeightedIntegerArray CellIndices;
	TMap<int32, TArray<int32>> JunctionIndices;
	Context.FilterCellInputData(MakeFilter(), CellIndices, JunctionIndices);

	CHECK_FALSE_MESSAGE(TEXT("A candidate carrying an already-placed unique tag must be excluded."), CellIndices.HasValue(0))
	CHECK_MESSAGE(TEXT("A candidate without the unique tag must still be selectable."), CellIndices.HasValue(1))
}

N_TEST_MEDIUM(FNFilterCellInputDataTests_FilterCellInputData_JunctionRotationConstraintExcludes,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData::JunctionRotationConstraintExcludes",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Confirms the junction-rotation gate is wired into the pipeline: an identity source requires a 180 yaw to
	// mate, so a junction whose matching window excludes 180 must drop out even though its socket size matches.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFilterCellInputDataHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("FilterTest"));
	FNVirtualCellData Cell = MakeCell();
	// Restrict the junction's matching yaw to a tight zero-centered window the required 180 cannot satisfy.
	Cell.Junctions[0].RotationConstraints.bEnforceMatchingRotation = true;
	Cell.Junctions[0].RotationConstraints.MinimumMatchingRotation = FRotator(0.f, -5.f, 0.f);
	Cell.Junctions[0].RotationConstraints.MaximumMatchingRotation = FRotator(0.f, 5.f, 0.f);
	Context.CellInputData.Add(Cell);

	FNWeightedIntegerArray CellIndices;
	TMap<int32, TArray<int32>> JunctionIndices;
	Context.FilterCellInputData(MakeFilter(), CellIndices, JunctionIndices);

	CHECK_FALSE_MESSAGE(TEXT("A junction whose rotation window excludes the required 180 must be filtered out."),
		CellIndices.HasData())
}

#endif //WITH_TESTS

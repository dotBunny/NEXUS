// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Cell/NTissueTagGroups.h"
#include "GameplayTagContainer.h"
#include "Types/NRawMeshUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNCheckGraphHarness
{
	// The built-in tag symbols are declared without an export macro, so they can't be linked from the editor
	// module; request them by their registered name instead (matches the strings in NWorldAssemblyGameplayTags.cpp).
	static FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	static FNAssemblyGraphBoneNode* MakeBoneRoot()
	{
		static FNVirtualBoneData BoneData;
		BoneData.SocketSize = FIntVector2(2, 4);
		return FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, FVector::ZeroVector, FRotator::ZeroRotator);
	}

	/** A minimal cell with deterministic bounds and a single junction so the node factory has something to copy. */
	static FNVirtualCellData MakeCell()
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-100.0), FVector(100.0));
		Cell.CellDetails.Hull = FNRawMeshUtils::MakeBoxHull(Cell.CellDetails.Bounds);
		Cell.Junctions.Add(0, FNCellJunctionDetails());
		return Cell;
	}

	/**
	 * Build a graph on Context holding CellCount cell nodes (plus the uncounted bone root). Cells supplies the
	 * backing storage for the node input data and must outlive Context; it is reserved up front so the node
	 * pointers into it stay stable.
	 */
	static void BuildGraph(FNVirtualOrganContext& Context, TArray<FNVirtualCellData>& Cells, const int32 CellCount)
	{
		Cells.Reserve(CellCount);
		Context.CellGraph = MakeUnique<FNAssemblyGraph>(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
		for (int32 i = 0; i < CellCount; i++)
		{
			FNVirtualCellData& Cell = Cells.Add_GetRef(MakeCell());
			FNAssemblyGraphCellNode* Node = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
			Context.CellGraph->RegisterNode(Node);
		}
	}
}

N_TEST_CRITICAL(FNCheckGraphTests_CheckGraph_NullGraphFails,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::CheckGraph::NullGraphFails",
	N_TEST_CONTEXT_ANYWHERE)
{
	// With no graph built at all the invariant check must fail rather than dereference a null graph.
	FNVirtualOrganContext Context(1234ull, TEXT("CheckGraphTest"));

	CHECK_FALSE_MESSAGE(TEXT("CheckGraph must fail when there is no graph."), Context.CheckGraph())
}

N_TEST_HIGH(FNCheckGraphTests_CheckGraph_BelowMinimumCellCountFails,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::CheckGraph::BelowMinimumCellCountFails",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A graph holding fewer cells than MinimumCellCount must be rejected.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCheckGraphHarness;

	TArray<FNVirtualCellData> Cells;
	FNVirtualOrganContext Context(1234ull, TEXT("CheckGraphTest"));
	Context.MinimumCellCount = 3;
	BuildGraph(Context, Cells, 2);

	CHECK_FALSE_MESSAGE(TEXT("Two cells must fail a minimum of three."), Context.CheckGraph())
}

N_TEST_HIGH(FNCheckGraphTests_CheckGraph_MeetsMinimumCellCountPasses,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::CheckGraph::MeetsMinimumCellCountPasses",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A graph that exactly meets the minimum (and has no maximum) must pass.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCheckGraphHarness;

	TArray<FNVirtualCellData> Cells;
	FNVirtualOrganContext Context(1234ull, TEXT("CheckGraphTest"));
	Context.MinimumCellCount = 2;
	BuildGraph(Context, Cells, 2);

	CHECK_MESSAGE(TEXT("Two cells must satisfy a minimum of two."), Context.CheckGraph())
}

N_TEST_HIGH(FNCheckGraphTests_CheckGraph_AboveMaximumCellCountFails,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::CheckGraph::AboveMaximumCellCountFails",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A graph holding more cells than MaximumCellCount must be rejected.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCheckGraphHarness;

	TArray<FNVirtualCellData> Cells;
	FNVirtualOrganContext Context(1234ull, TEXT("CheckGraphTest"));
	Context.MaximumCellCount = 2;
	BuildGraph(Context, Cells, 3);

	CHECK_FALSE_MESSAGE(TEXT("Three cells must fail a maximum of two."), Context.CheckGraph())
}

N_TEST_MEDIUM(FNCheckGraphTests_CheckGraph_WithinCellCountBoundsPasses,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::CheckGraph::WithinCellCountBoundsPasses",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A cell count comfortably inside both bounds must pass.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCheckGraphHarness;

	TArray<FNVirtualCellData> Cells;
	FNVirtualOrganContext Context(1234ull, TEXT("CheckGraphTest"));
	Context.MinimumCellCount = 1;
	Context.MaximumCellCount = 5;
	BuildGraph(Context, Cells, 3);

	CHECK_MESSAGE(TEXT("Three cells must pass a 1..5 bound."), Context.CheckGraph())
}

N_TEST_HIGH(FNCheckGraphTests_ValidateGraph_ReflectsCheckAndSetsSuccessful,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::ValidateGraph::ReflectsCheckAndSetsSuccessful",
	N_TEST_CONTEXT_ANYWHERE)
{
	// ValidateGraph must return the same verdict as CheckGraph and latch it onto IsSuccessful, for both a
	// failing and a passing configuration of the same graph.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCheckGraphHarness;

	TArray<FNVirtualCellData> Cells;
	FNVirtualOrganContext Context(1234ull, TEXT("CheckGraphTest"));
	BuildGraph(Context, Cells, 2);

	// Failing: minimum of three against a two-cell graph.
	Context.MinimumCellCount = 3;
	CHECK_FALSE_MESSAGE(TEXT("ValidateGraph must report failure below the minimum."), Context.ValidateGraph())
	CHECK_FALSE_MESSAGE(TEXT("IsSuccessful must be false after a failed validation."), Context.IsSuccessful())

	// Passing: relax the minimum so the same graph validates.
	Context.MinimumCellCount = 1;
	CHECK_MESSAGE(TEXT("ValidateGraph must report success within bounds."), Context.ValidateGraph())
	CHECK_MESSAGE(TEXT("IsSuccessful must be true after a successful validation."), Context.IsSuccessful())
}

N_TEST_HIGH(FNCheckGraphTests_CheckGraph_RequiredAnyTagsEnforced,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::CheckGraph::RequiredAnyTagsEnforced",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Required-any enforcement: every configured required-any tag must actually appear in the placed set, so a
	// graph that placed nothing for a required group (or only an unrelated tag) is rejected, and one that placed
	// the required tag passes. Guards the corrected comparison (placed-contains-configured) against regressing
	// back to the no-op direction where a required group could be left unsatisfied.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCheckGraphHarness;

	const FGameplayTag Configured = Tag(TEXT("NEXUS.WorldAssembly.Behavior.Starter"));
	const FGameplayTag Foreign = Tag(TEXT("NEXUS.WorldAssembly.Behavior.Finisher"));

	TArray<FNVirtualCellData> Cells;
	FNVirtualOrganContext Context(1234ull, TEXT("CheckGraphTest"));
	Context.MinimumCellCount = 1;
	BuildGraph(Context, Cells, 2);
	Context.CellInputDataSummary.GroupTags.RequiredAnyTags.AppendTags(FGameplayTagContainer(Configured));

	// Nothing placed for the required group: the requirement is unmet and the graph must be rejected.
	CHECK_FALSE_MESSAGE(TEXT("A required-any group with nothing placed must fail the check."),
		Context.CheckGraph())

	// Only an unrelated tag placed: still does not satisfy the configured requirement.
	Context.PlacedTagGroups.RequiredAnyTags.AppendTags(FGameplayTagContainer(Foreign));
	CHECK_FALSE_MESSAGE(TEXT("An unrelated placed tag must not satisfy a required-any group."),
		Context.CheckGraph())

	// The configured required tag is now placed: the requirement is satisfied.
	Context.PlacedTagGroups.RequiredAnyTags.AppendTags(FGameplayTagContainer(Configured));
	CHECK_MESSAGE(TEXT("Placing the configured required-any tag must satisfy the check."),
		Context.CheckGraph())
}

#endif //WITH_TESTS

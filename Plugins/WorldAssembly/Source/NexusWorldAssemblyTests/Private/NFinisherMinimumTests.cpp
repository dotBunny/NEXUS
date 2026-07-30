// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/NAssemblyOperationSettings.h"
#include "Assembly/NAssemblyTaskAnalytics.h"
#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
#include "Assembly/Contexts/NPassContext.h"
#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Assembly/Contexts/NVirtualWorldContext.h"
#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphBoneNode.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Assembly/Tasks/NOrganGraphBuilderTask.h"
#include "Cell/NCellJunctionDetails.h"
#include "GameplayTagContainer.h"
#include "Math/NMersenneTwister.h"
#include "Types/NRawMeshUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNFinisherMinimumHarness
{
	// The built-in tag symbols are declared without an export macro, so they can't be linked from the editor module;
	// request them by their registered name instead (matches the strings in NWorldAssemblyGameplayTags.cpp).
	static FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	static FGameplayTag Finisher() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.Finisher")); }
	static FGameplayTag FinisherOnly() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.FinisherOnly")); }
	/** An arbitrary valid tag used to stand in for a configured Unique + RequiredAny group member. */
	static FGameplayTag GroupTag() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.Starter")); }

	/**
	 * A cell with deterministic box bounds/hull and a single junction (key 0) at JunctionOffset (cell-local). Placing
	 * the junction away from the origin lets a placed candidate land clear of a starter sitting at the origin.
	 */
	static FNVirtualCellData MakeCell(const FVector& JunctionOffset = FVector::ZeroVector)
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-100.0), FVector(100.0));
		Cell.CellDetails.Hull = FNRawMeshUtils::MakeBoxHull(Cell.CellDetails.Bounds);
		FNCellJunctionDetails Junction;
		Junction.WorldLocation = JunctionOffset;
		Cell.Junctions.Add(0, Junction);
		return Cell;
	}

	static FNAssemblyGraphBoneNode* MakeBoneRoot()
	{
		static FNVirtualBoneData BoneData;
		BoneData.SocketSize = FIntVector2(2, 4);
		return FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, FVector::ZeroVector, FRotator::ZeroRotator);
	}
}

//
// IsUnmetFinisherMinimum predicate (pure)
//

N_TEST_HIGH(FNFinisherMinimumTests_Predicate_FinisherOnlyShortOfMinimumIsTarget,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FinisherMinimum::FinisherOnlyShortOfMinimumIsTarget",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The case from the bug report: a FinisherOnly cell with MinimumCount 1 and nothing placed yet is a target.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFinisherMinimumHarness;

	FNVirtualCellData Cell = MakeCell();
	Cell.AssemblyTags = FGameplayTagContainer(FinisherOnly());
	Cell.MinimumCount = 1;
	Cell.UsedCount = 0;

	CHECK_MESSAGE(TEXT("A FinisherOnly cell short of its MinimumCount must be a guarantee-pass target."),
		FNVirtualOrganContext::IsUnmetFinisherMinimum(Cell, FGameplayTagContainer()))
}

N_TEST_HIGH(FNFinisherMinimumTests_Predicate_FinisherTagShortOfMinimumIsTarget,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FinisherMinimum::FinisherTagShortOfMinimumIsTarget",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A plain Finisher (not -Only) that is still short after normal expansion is also a target.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFinisherMinimumHarness;

	FNVirtualCellData Cell = MakeCell();
	Cell.AssemblyTags = FGameplayTagContainer(Finisher());
	Cell.MinimumCount = 2;
	Cell.UsedCount = 1;

	CHECK_MESSAGE(TEXT("A Finisher cell still below its MinimumCount must be a target."),
		FNVirtualOrganContext::IsUnmetFinisherMinimum(Cell, FGameplayTagContainer()))
}

N_TEST_MEDIUM(FNFinisherMinimumTests_Predicate_MetMinimumIsNotTarget,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FinisherMinimum::MetMinimumIsNotTarget",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Once the minimum is met the cell needs no forcing.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFinisherMinimumHarness;

	FNVirtualCellData Cell = MakeCell();
	Cell.AssemblyTags = FGameplayTagContainer(FinisherOnly());
	Cell.MinimumCount = 1;
	Cell.UsedCount = 1;

	CHECK_FALSE_MESSAGE(TEXT("A finisher that already meets its minimum must not be a target."),
		FNVirtualOrganContext::IsUnmetFinisherMinimum(Cell, FGameplayTagContainer()))
}

N_TEST_MEDIUM(FNFinisherMinimumTests_Predicate_NonFinisherIsNotTarget,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FinisherMinimum::NonFinisherIsNotTarget",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A non-finisher cell cannot be placed at cap time and is handled by normal expansion, so it is never a target
	// of this pass even when short of its minimum.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFinisherMinimumHarness;

	FNVirtualCellData Cell = MakeCell();
	Cell.MinimumCount = 1;
	Cell.UsedCount = 0;

	CHECK_FALSE_MESSAGE(TEXT("An untagged cell must not be a finisher-minimum target."),
		FNVirtualOrganContext::IsUnmetFinisherMinimum(Cell, FGameplayTagContainer()))
}

N_TEST_MEDIUM(FNFinisherMinimumTests_Predicate_UnsatisfiableMinimumIsNotTarget,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FinisherMinimum::UnsatisfiableMinimumIsNotTarget",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A minimum above a positive maximum can never be reached; CheckGraph skips it, so the pass must not chase it.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFinisherMinimumHarness;

	FNVirtualCellData Cell = MakeCell();
	Cell.AssemblyTags = FGameplayTagContainer(FinisherOnly());
	Cell.MinimumCount = 2;
	Cell.MaximumCount = 1;
	Cell.UsedCount = 0;

	CHECK_FALSE_MESSAGE(TEXT("A minimum greater than a positive maximum must not be a target."),
		FNVirtualOrganContext::IsUnmetFinisherMinimum(Cell, FGameplayTagContainer()))
}

N_TEST_MEDIUM(FNFinisherMinimumTests_Predicate_UniqueAndRequiredGovernedIsNotTarget,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FinisherMinimum::UniqueAndRequiredGovernedIsNotTarget",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Cells governed by a combined Unique + RequiredAny group are validated by the RequiredAny check, not their
	// per-cell minimum (mirrors CheckGraph), so they are excluded from the target set.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFinisherMinimumHarness;

	FNVirtualCellData Cell = MakeCell();
	FGameplayTagContainer CellTags(FinisherOnly());
	CellTags.AddTag(GroupTag());
	Cell.AssemblyTags = CellTags;
	Cell.MinimumCount = 1;
	Cell.UsedCount = 0;

	CHECK_FALSE_MESSAGE(TEXT("A cell in a Unique + RequiredAny group must be excluded from forcing."),
		FNVirtualOrganContext::IsUnmetFinisherMinimum(Cell, FGameplayTagContainer(GroupTag())))
}

//
// EnsureFinisherMinimums guarantee pass (integration — drives the real builder placement)
//

namespace NEXUS::UnitTests::NWorldAssembly::FNFinisherMinimumHarness
{
	/** Construct a builder task over OrganContext with throwaway supporting contexts (no world, no collision). */
	static TUniquePtr<FNOrganGraphBuilderTask> MakeTask(const TSharedPtr<FNVirtualOrganContext>& OrganContext)
	{
		static FNAssemblyOperationSettings Settings;
		const TSharedPtr<FNPassContext> PassContext = MakeShared<FNPassContext>();
		const TArray<FBoxSphereBounds> NoBounds;
		const TSharedPtr<FNVirtualWorldContext> WorldContext = MakeShared<FNVirtualWorldContext>(static_cast<UWorld*>(nullptr), NoBounds, Settings);
		int32 Ticket = 0;
		const TSharedPtr<FNAssemblyTaskGraphContext> TaskGraphContext = MakeShared<FNAssemblyTaskGraphContext>(static_cast<UWorld*>(nullptr), Ticket, Settings);
		const TSharedPtr<FNAssemblyTaskAnalytics> Analytics = MakeShared<FNAssemblyTaskAnalytics, ESPMode::ThreadSafe>(FText::FromString(TEXT("FinisherMinimumTest")));
		return MakeUnique<FNOrganGraphBuilderTask>(OrganContext, PassContext, WorldContext, TaskGraphContext, Analytics);
	}
}

N_TEST_CRITICAL(FNFinisherMinimumTests_Pass_PlacesFinisherOnlyOntoOpenJunction,
	"NEXUS::UnitTests::NWorldAssembly::FNOrganGraphBuilderTask::FinisherMinimum::PlacesFinisherOnlyOntoOpenJunction",
	N_TEST_CONTEXT_ANYWHERE)
{
	// End-to-end: a FinisherOnly cell with MinimumCount 1 that the rest of the build never placed is forced onto a
	// remaining open junction, after which the graph validates. This is the fix for the reported failure.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFinisherMinimumHarness;

	const TSharedPtr<FNVirtualOrganContext> Context = MakeShared<FNVirtualOrganContext>(4242ull, FString(TEXT("FinisherMinimumTest")));
	Context->bUnbound = true; // Keep the focus on placement, not organ-bounds rejection.

	// index 0: the starter, with one open junction parked far from the origin so the finisher lands clear of it.
	Context->CellInputData.Add(MakeCell(FVector(1000.0, 0.0, 0.0)));
	// index 1: the FinisherOnly cell that must appear at least once.
	FNVirtualCellData FinisherCell = MakeCell();
	FinisherCell.AssemblyTags = FGameplayTagContainer(FinisherOnly());
	FinisherCell.MinimumCount = 1;
	Context->CellInputData.Add(FinisherCell);
	Context->CellInputDataSummary.bFoundFinisherOnlyTagged = true;

	// A graph with the starter registered and its junction still open.
	Context->CellGraph = MakeUnique<FNAssemblyGraph>(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* Starter = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Context->CellInputData[0], FVector(100.f));
	Context->CellGraph->RegisterNode(Starter);

	// Precondition: unplaced finisher means the graph currently fails its per-cell minimum (the bug).
	CHECK_EQUALS("The finisher should start unplaced.", Context->CellInputData[1].UsedCount, 0)
	CHECK_FALSE_MESSAGE(TEXT("Before the guarantee pass the graph must fail its finisher minimum."), Context->CheckGraph())

	const TUniquePtr<FNOrganGraphBuilderTask> Task = MakeTask(Context);
	FNMersenneTwister Random(4242ull);
	Task->EnsureFinisherMinimums(Random);

	CHECK_MESSAGE(TEXT("The guarantee pass must place the FinisherOnly cell up to its MinimumCount."),
		Context->CellInputData[1].UsedCount >= Context->CellInputData[1].MinimumCount)
	CHECK_MESSAGE(TEXT("The graph must validate once the finisher minimum is satisfied."), Context->CheckGraph())
}

N_TEST_HIGH(FNFinisherMinimumTests_Pass_NoOpenJunctionLeavesFinisherUnplaced,
	"NEXUS::UnitTests::NWorldAssembly::FNOrganGraphBuilderTask::FinisherMinimum::NoOpenJunctionLeavesFinisherUnplaced",
	N_TEST_CONTEXT_ANYWHERE)
{
	// When the layout offers no open junction the cell genuinely cannot be hosted: the pass places nothing and the
	// graph still fails, which correctly drives the build to retry rather than fabricating an invalid placement.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFinisherMinimumHarness;

	const TSharedPtr<FNVirtualOrganContext> Context = MakeShared<FNVirtualOrganContext>(4242ull, FString(TEXT("FinisherMinimumTest")));
	Context->bUnbound = true;

	Context->CellInputData.Add(MakeCell());
	FNVirtualCellData FinisherCell = MakeCell();
	FinisherCell.AssemblyTags = FGameplayTagContainer(FinisherOnly());
	FinisherCell.MinimumCount = 1;
	Context->CellInputData.Add(FinisherCell);
	Context->CellInputDataSummary.bFoundFinisherOnlyTagged = true;

	// Starter's only junction is consumed by the bone root, so the graph has no open junction to host the finisher.
	FNAssemblyGraphBoneNode* Bone = MakeBoneRoot();
	Context->CellGraph = MakeUnique<FNAssemblyGraph>(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* Starter = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Context->CellInputData[0], FVector(100.f));
	Context->CellGraph->RegisterNode(Starter);
	Starter->LinkJunction(0, Bone);

	const TUniquePtr<FNOrganGraphBuilderTask> Task = MakeTask(Context);
	FNMersenneTwister Random(4242ull);
	Task->EnsureFinisherMinimums(Random);

	CHECK_EQUALS("With no open junction the finisher must remain unplaced.", Context->CellInputData[1].UsedCount, 0)
	CHECK_FALSE_MESSAGE(TEXT("An unplaceable finisher minimum must still fail validation so the build retries."), Context->CheckGraph())
}

N_TEST_HIGH(FNFinisherMinimumTests_Pass_NoTargetsIsNoOp,
	"NEXUS::UnitTests::NWorldAssembly::FNOrganGraphBuilderTask::FinisherMinimum::NoTargetsIsNoOp",
	N_TEST_CONTEXT_ANYWHERE)
{
	// With no finisher-minimum cell in the pool the pass must not touch the graph at all, so tissues that do not use
	// the feature keep identical output.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNFinisherMinimumHarness;

	const TSharedPtr<FNVirtualOrganContext> Context = MakeShared<FNVirtualOrganContext>(4242ull, FString(TEXT("FinisherMinimumTest")));
	Context->bUnbound = true;

	// A FinisherOnly cell with no minimum, plus a plain candidate — neither is a target.
	FNVirtualCellData FinisherCell = MakeCell();
	FinisherCell.AssemblyTags = FGameplayTagContainer(FinisherOnly());
	Context->CellInputData.Add(FinisherCell);
	Context->CellInputData.Add(MakeCell(FVector(1000.0, 0.0, 0.0)));
	Context->CellInputDataSummary.bFoundFinisherOnlyTagged = true;

	Context->CellGraph = MakeUnique<FNAssemblyGraph>(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* Starter = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Context->CellInputData[1], FVector(100.f));
	Context->CellGraph->RegisterNode(Starter);

	const int32 CellCountBefore = Context->CellGraph->GetCellNodeCount();

	const TUniquePtr<FNOrganGraphBuilderTask> Task = MakeTask(Context);
	FNMersenneTwister Random(4242ull);
	Task->EnsureFinisherMinimums(Random);

	CHECK_EQUALS("A pool with no finisher-minimum target must leave the graph untouched.",
		Context->CellGraph->GetCellNodeCount(), CellCountBefore)
}

#endif //WITH_TESTS

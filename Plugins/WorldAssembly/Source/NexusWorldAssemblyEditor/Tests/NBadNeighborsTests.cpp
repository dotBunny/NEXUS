// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Cell/NTissueTagGroups.h"
#include "GameplayTagContainer.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNBadNeighborsHarness
{
	// The built-in tag symbols are declared without an export macro, so they can't be linked from the editor
	// module; request them by their registered name instead (matches the strings in NWorldAssemblyGameplayTags.cpp).
	static FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// Three distinct registered tags standing in as independent "bad-neighbor group" markers. Their built-in
	// meaning is irrelevant here — they are only used because they are guaranteed-registered, distinct leaves.
	static FGameplayTag GroupRed() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.Starter")); }
	static FGameplayTag GroupBlue() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.Finisher")); }
	static FGameplayTag Unrelated() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.NotStarter")); }

	/** A tag-group registry whose BadNeighbors set marks the supplied tags as bad-neighbor groups. */
	static FNTissueTagGroups MakeRegistry(const FGameplayTagContainer& BadNeighborGroups)
	{
		FNTissueTagGroups Groups;
		Groups.AppendBadNeighborsTags(BadNeighborGroups);
		return Groups;
	}

	/** Build a cell with a single junction so the node factory has something to copy, carrying the given tags. */
	static FNVirtualCellData MakeCell(const FGameplayTagContainer& AssemblyTags)
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-100.0), FVector(100.0));
		Cell.Junctions.Add(0, FNCellJunctionDetails());
		Cell.AssemblyTags = AssemblyTags;
		return Cell;
	}

	/**
	 * Owns a graph + bone + a single placed source cell node whose cached AssemblyTags carry SourceTags, so the
	 * production gate can resolve the source's bad-neighbor groups straight off a real node (exactly as
	 * FilterCellInputData does via Filter.SourceCellNode->GetAssemblyTags()). The graph owns the bone and the
	 * registered node and frees them on destruction.
	 */
	struct FSourceFixture
	{
		FNVirtualBoneData BoneData;
		FNVirtualCellData SourceCell;
		TUniquePtr<FNAssemblyGraph> Graph;
		FNAssemblyGraphCellNode* SourceNode = nullptr;

		explicit FSourceFixture(const FGameplayTagContainer& SourceTags)
			: SourceCell(MakeCell(SourceTags))
		{
			BoneData.SocketSize = FIntVector2(2, 4);
			FNAssemblyGraphBoneNode* Bone = FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, FVector::ZeroVector, FRotator::ZeroRotator);
			Graph = MakeUnique<FNAssemblyGraph>(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
			SourceNode = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &SourceCell, FVector(100.f));
			Graph->RegisterNode(SourceNode);
		}
	};

	/**
	 * Drives the real production gate: resolves the source's bad-neighbor groups off SourceNode (pass nullptr to
	 * model the null-source pre-filter), then asks whether Candidate shares one. Any change to the production
	 * comparison surfaces directly through these two calls.
	 */
	static bool IsRejected(const FNTissueTagGroups& Registry, FNAssemblyGraphCellNode* SourceNode, const FNVirtualCellData& Candidate)
	{
		const FGameplayTagContainer SourceBadNeighborTags = FNVirtualOrganContext::ResolveSourceBadNeighborTags(Registry, SourceNode);
		return FNVirtualOrganContext::IsBadNeighbor(SourceBadNeighborTags, Candidate);
	}
}

N_TEST_CRITICAL(FNBadNeighborsTests_BadNeighbors_SameGroupRejected,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::BadNeighbors::SameGroupRejected",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the core rule: a candidate sharing a bad-neighbor group with the source cell is rejected so the
	// two can never be placed beside each other.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNBadNeighborsHarness;

	const FNTissueTagGroups Registry = MakeRegistry(FGameplayTagContainer(GroupRed()));
	FSourceFixture Source{FGameplayTagContainer(GroupRed())};
	const FNVirtualCellData Candidate = MakeCell(FGameplayTagContainer(GroupRed()));

	CHECK_MESSAGE(TEXT("Candidate in the same bad-neighbor group as the source must be rejected."),
		IsRejected(Registry, Source.SourceNode, Candidate))
}

N_TEST_HIGH(FNBadNeighborsTests_BadNeighbors_DifferentGroupAllowed,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::BadNeighbors::DifferentGroupAllowed",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two cells that each belong to a bad-neighbor group, but different ones, are free to sit next to each other.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNBadNeighborsHarness;

	FGameplayTagContainer BothGroups;
	BothGroups.AddTag(GroupRed());
	BothGroups.AddTag(GroupBlue());
	const FNTissueTagGroups Registry = MakeRegistry(BothGroups);

	FSourceFixture Source{FGameplayTagContainer(GroupRed())};
	const FNVirtualCellData Candidate = MakeCell(FGameplayTagContainer(GroupBlue()));

	CHECK_FALSE_MESSAGE(TEXT("Candidate in a different bad-neighbor group than the source must be allowed."),
		IsRejected(Registry, Source.SourceNode, Candidate))
}

N_TEST_HIGH(FNBadNeighborsTests_BadNeighbors_SourceNotInGroupAllowed,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::BadNeighbors::SourceNotInGroupAllowed",
	N_TEST_CONTEXT_ANYWHERE)
{
	// When the source cell belongs to no bad-neighbor group, nothing it sits beside can be rejected on that
	// basis — even a candidate that is itself a bad-neighbor group member.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNBadNeighborsHarness;

	const FNTissueTagGroups Registry = MakeRegistry(FGameplayTagContainer(GroupRed()));
	FSourceFixture Source{FGameplayTagContainer(Unrelated())};
	const FNVirtualCellData Candidate = MakeCell(FGameplayTagContainer(GroupRed()));

	CHECK_FALSE_MESSAGE(TEXT("A source with no bad-neighbor tags must not reject any candidate."),
		IsRejected(Registry, Source.SourceNode, Candidate))
}

N_TEST_MEDIUM(FNBadNeighborsTests_BadNeighbors_EmptyRegistryNeverRejects,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::BadNeighbors::EmptyRegistryNeverRejects",
	N_TEST_CONTEXT_ANYWHERE)
{
	// With no bad-neighbor groups configured the gate is skipped entirely, even when source and candidate
	// share a tag that would otherwise mark them as a group.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNBadNeighborsHarness;

	const FNTissueTagGroups Registry = MakeRegistry(FGameplayTagContainer());
	FSourceFixture Source{FGameplayTagContainer(GroupRed())};
	const FNVirtualCellData Candidate = MakeCell(FGameplayTagContainer(GroupRed()));

	CHECK_FALSE_MESSAGE(TEXT("An empty bad-neighbor registry must never reject a candidate."),
		IsRejected(Registry, Source.SourceNode, Candidate))
}

N_TEST_MEDIUM(FNBadNeighborsTests_BadNeighbors_NullSourceNeverRejects,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::BadNeighbors::NullSourceNeverRejects",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The start-node pre-filter runs with no source node; production passes Filter.SourceCellNode == nullptr,
	// which must skip the gate so the start cell is never rejected as a bad neighbour of nothing.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNBadNeighborsHarness;

	const FNTissueTagGroups Registry = MakeRegistry(FGameplayTagContainer(GroupRed()));
	const FNVirtualCellData Candidate = MakeCell(FGameplayTagContainer(GroupRed()));

	CHECK_FALSE_MESSAGE(TEXT("A null source (start pre-filter) must never reject a candidate."),
		IsRejected(Registry, nullptr, Candidate))
}

N_TEST_HIGH(FNBadNeighborsTests_BadNeighbors_RealNodeTagsDriveRejection,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::BadNeighbors::RealNodeTagsDriveRejection",
	N_TEST_CONTEXT_ANYWHERE)
{
	// End-to-end wiring: a placed cell node caches its AssemblyTags at construction, and the production gate
	// reads them back through GetAssemblyTags() inside ResolveSourceBadNeighborTags. Confirms the resolved
	// groups are non-empty and that they drive both the reject and the allow decision off the node's own tags.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNBadNeighborsHarness;

	const FNTissueTagGroups Registry = MakeRegistry(FGameplayTagContainer(GroupRed()));
	FSourceFixture Source{FGameplayTagContainer(GroupRed())};

	const FGameplayTagContainer SourceBadNeighborTags = FNVirtualOrganContext::ResolveSourceBadNeighborTags(Registry, Source.SourceNode);

	const FNVirtualCellData SameGroup = MakeCell(FGameplayTagContainer(GroupRed()));
	const FNVirtualCellData OtherTag = MakeCell(FGameplayTagContainer(Unrelated()));

	CHECK_MESSAGE(TEXT("The node's cached tags should resolve to its bad-neighbor group."),
		!SourceBadNeighborTags.IsEmpty())
	CHECK_MESSAGE(TEXT("A same-group candidate must be rejected using the node's own tags."),
		FNVirtualOrganContext::IsBadNeighbor(SourceBadNeighborTags, SameGroup))
	CHECK_FALSE_MESSAGE(TEXT("A candidate outside the node's bad-neighbor group must be allowed."),
		FNVirtualOrganContext::IsBadNeighbor(SourceBadNeighborTags, OtherTag))
}

#endif //WITH_TESTS

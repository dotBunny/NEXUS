// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Contexts/NAssemblyOperationContext.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"
#include "Engine/TriggerVolume.h"
#include "Organ/NOrganComponent.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContextHarness
{
	/**
	 * Spawn a volume-owned organ and seed the context's OrganData with it, mirroring the post-AddOrganComponent
	 * state that LockAndPreprocess consumes. The organ is hosted on an ATriggerVolume so IsVolumeBased() is true.
	 * @return The registered organ, or nullptr when the host volume could not be spawned.
	 */
	static UNOrganComponent* AddOrgan(UWorld* World, FNAssemblyOperationContext& Context, const bool bUnbound)
	{
		ATriggerVolume* Volume = World->SpawnActor<ATriggerVolume>(FVector::ZeroVector, FRotator::ZeroRotator);
		if (Volume == nullptr)
		{
			return nullptr;
		}

		UNOrganComponent* Organ = NewObject<UNOrganComponent>(Volume);
		Volume->AddInstanceComponent(Organ);
		Organ->RegisterComponent();
		Organ->bUnbound = bUnbound;

		Context.InputComponents.Add(Organ);
		FNWorldOrganData& Data = Context.OrganData.Add(Organ, FNWorldOrganData());
		Data.SourceComponent = Organ;
		return Organ;
	}

	/**
	 * Record Child as fully contained by Parent, exactly as FNAssemblyOperationContext::AddOrganComponent would:
	 * containment is derived from physical volume bounds and is recorded regardless of the child's bUnbound flag.
	 */
	static void MarkContained(FNAssemblyOperationContext& Context, UNOrganComponent* Parent, UNOrganComponent* Child)
	{
		Context.OrganData[Parent].ContainedComponents.AddUnique(Child);
	}

	/** @return The generation-order phase Organ landed in, or INDEX_NONE when it was dropped from the order entirely. */
	static int32 PhaseOf(const FNAssemblyOperationContext& Context, UNOrganComponent* Organ)
	{
		for (int32 Phase = 0; Phase < Context.GenerationOrder.Num(); Phase++)
		{
			if (Context.GenerationOrder[Phase].Contains(Organ))
			{
				return Phase;
			}
		}
		return INDEX_NONE;
	}
}

N_TEST_HIGH(FNAssemblyOperationContextTests_GenerationOrder_BoundedContainerWithUnboundChildIsScheduled,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContext::GenerationOrder::BoundedContainerWithUnboundChildIsScheduled",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies a bounded organ that encloses an unbound organ is still scheduled, and ahead of that child. The
	// unbound child is never marked processed (it is deferred to its own trailing pass), so before the fix it left
	// the container permanently unschedulable, which surfaced as a false "circular cycle" and a dropped organ.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContextHarness;
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNAssemblyOperationContext Context(1);
		UNOrganComponent* Parent = AddOrgan(World, Context, false);
		UNOrganComponent* Child = AddOrgan(World, Context, true);
		if (Parent == nullptr || Child == nullptr)
		{
			ADD_ERROR("Could not spawn organ volumes for the test.");
			return;
		}
		MarkContained(Context, Parent, Child);

		Context.LockAndPreprocess(World);

		const int32 ParentPhase = PhaseOf(Context, Parent);
		const int32 ChildPhase = PhaseOf(Context, Child);

		CHECK_MESSAGE(TEXT("The bounded container must appear in the generation order."), ParentPhase != INDEX_NONE)
		CHECK_MESSAGE(TEXT("The unbound child must appear in the generation order."), ChildPhase != INDEX_NONE)
		CHECK_MESSAGE(TEXT("The unbound child must be scheduled after its container."),
			ParentPhase != INDEX_NONE && ChildPhase != INDEX_NONE && ParentPhase < ChildPhase)
	});
}

N_TEST_MEDIUM(FNAssemblyOperationContextTests_GenerationOrder_ContainerWithBoundAndUnboundChildren,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContext::GenerationOrder::ContainerWithBoundAndUnboundChildren",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies mixed containment orders correctly: a bound child generates before its container, the container
	// before the unbound child, and the unbound child lands last in its own pass.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContextHarness;
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNAssemblyOperationContext Context(1);
		UNOrganComponent* Container = AddOrgan(World, Context, false);
		UNOrganComponent* BoundChild = AddOrgan(World, Context, false);
		UNOrganComponent* UnboundChild = AddOrgan(World, Context, true);
		if (Container == nullptr || BoundChild == nullptr || UnboundChild == nullptr)
		{
			ADD_ERROR("Could not spawn organ volumes for the test.");
			return;
		}
		MarkContained(Context, Container, BoundChild);
		MarkContained(Context, Container, UnboundChild);

		Context.LockAndPreprocess(World);

		const int32 ContainerPhase = PhaseOf(Context, Container);
		const int32 BoundChildPhase = PhaseOf(Context, BoundChild);
		const int32 UnboundChildPhase = PhaseOf(Context, UnboundChild);

		CHECK_MESSAGE(TEXT("All three organs must appear in the generation order."),
			ContainerPhase != INDEX_NONE && BoundChildPhase != INDEX_NONE && UnboundChildPhase != INDEX_NONE)
		CHECK_MESSAGE(TEXT("The bound child must be scheduled before its container."),
			BoundChildPhase != INDEX_NONE && ContainerPhase != INDEX_NONE && BoundChildPhase < ContainerPhase)
		CHECK_MESSAGE(TEXT("The unbound child must be scheduled after its container."),
			ContainerPhase != INDEX_NONE && UnboundChildPhase != INDEX_NONE && ContainerPhase < UnboundChildPhase)
	});
}

N_TEST_MEDIUM(FNAssemblyOperationContextTests_GenerationOrder_ContainerOfOnlyUnboundQualifiesForFirstPhase,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContext::GenerationOrder::ContainerOfOnlyUnboundQualifiesForFirstPhase",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies the phase-0 fast path: an organ whose only contained organ is unbound has no real dependency, so it
	// belongs in the first parallel phase rather than being held back (only bound contained organs gate ordering).
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContextHarness;
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNAssemblyOperationContext Context(1);
		UNOrganComponent* Parent = AddOrgan(World, Context, false);
		UNOrganComponent* Child = AddOrgan(World, Context, true);
		if (Parent == nullptr || Child == nullptr)
		{
			ADD_ERROR("Could not spawn organ volumes for the test.");
			return;
		}
		MarkContained(Context, Parent, Child);

		Context.LockAndPreprocess(World);

		CHECK_EQUALS("A container of only unbound organs belongs in the first phase.", PhaseOf(Context, Parent), 0)
		CHECK_MESSAGE(TEXT("The unbound child must still be scheduled after its container."),
			PhaseOf(Context, Child) > PhaseOf(Context, Parent))
	});
}

N_TEST_MEDIUM(FNAssemblyOperationContextTests_GenerationOrder_BoundedNestingOrdersInnerBeforeOuter,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContext::GenerationOrder::BoundedNestingOrdersInnerBeforeOuter",
	N_TEST_CONTEXT_EDITOR)
{
	// Guards the unchanged path: ordinary bounded nesting still generates the inner organ before the outer one.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContextHarness;
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNAssemblyOperationContext Context(1);
		UNOrganComponent* Outer = AddOrgan(World, Context, false);
		UNOrganComponent* Inner = AddOrgan(World, Context, false);
		if (Outer == nullptr || Inner == nullptr)
		{
			ADD_ERROR("Could not spawn organ volumes for the test.");
			return;
		}
		MarkContained(Context, Outer, Inner);

		Context.LockAndPreprocess(World);

		const int32 OuterPhase = PhaseOf(Context, Outer);
		const int32 InnerPhase = PhaseOf(Context, Inner);

		CHECK_MESSAGE(TEXT("Both nested organs must appear in the generation order."),
			OuterPhase != INDEX_NONE && InnerPhase != INDEX_NONE)
		CHECK_MESSAGE(TEXT("The inner organ must be scheduled before the outer container."),
			OuterPhase != INDEX_NONE && InnerPhase != INDEX_NONE && InnerPhase < OuterPhase)
	});
}

#endif //WITH_TESTS

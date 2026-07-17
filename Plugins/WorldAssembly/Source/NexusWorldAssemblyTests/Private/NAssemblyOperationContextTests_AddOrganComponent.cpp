// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Contexts/NAssemblyOperationContext.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"
#include "Engine/TriggerVolume.h"
#include "GameFramework/Actor.h"
#include "Organ/NOrganComponent.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContextAddOrganHarness
{
	/** Attach a freshly registered organ component to Owner and return it. */
	static UNOrganComponent* AttachOrgan(AActor* Owner, const bool bUnbound)
	{
		UNOrganComponent* Organ = NewObject<UNOrganComponent>(Owner);
		Owner->AddInstanceComponent(Organ);
		Organ->RegisterComponent();
		Organ->bUnbound = bUnbound;
		return Organ;
	}

	/** Spawn a volume-based organ (owner IsA AVolume, so IsVolumeBased() holds). */
	static UNOrganComponent* SpawnVolumeOrgan(UWorld* World, const bool bUnbound = false)
	{
		ATriggerVolume* Volume = World->SpawnActor<ATriggerVolume>(FVector::ZeroVector, FRotator::ZeroRotator);
		return Volume != nullptr ? AttachOrgan(Volume, bUnbound) : nullptr;
	}

	/** Spawn an organ whose owner is a plain actor (not a volume), so IsVolumeBased() is false. */
	static UNOrganComponent* SpawnNonVolumeOrgan(UWorld* World)
	{
		AActor* Actor = World->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator);
		return Actor != nullptr ? AttachOrgan(Actor, false) : nullptr;
	}
}

N_TEST_CRITICAL(FNAssemblyOperationContextTests_AddOrganComponent_NonVolumeOrganInLevelDoesNotCrash,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContext::AddOrganComponent::NonVolumeOrganInLevelDoesNotCrash",
	N_TEST_CONTEXT_EDITOR)
{
	// Adding a bounded organ scans every organ in the level for spatial relationships. An organ whose owner is not a
	// volume has no bounds; before the null guard this dereferenced a null cast and crashed. It must now be skipped.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContextAddOrganHarness;
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNOrganComponent* VolumeOrgan = SpawnVolumeOrgan(World, false);
		UNOrganComponent* NonVolumeOrgan = SpawnNonVolumeOrgan(World);
		if (VolumeOrgan == nullptr || NonVolumeOrgan == nullptr)
		{
			ADD_ERROR("Could not spawn the test organs.");
			return;
		}

		FNAssemblyOperationContext Context(1);
		const bool bAdded = Context.AddOrganComponent(VolumeOrgan);

		CHECK_MESSAGE(TEXT("Adding the volume organ must succeed without crashing on the non-volume organ."), bAdded)
		CHECK_MESSAGE(TEXT("The volume organ must be recorded."), Context.OrganData.Contains(VolumeOrgan))
		CHECK_FALSE_MESSAGE(TEXT("A non-volume organ has no bounds and must not be pulled into the relationship graph."),
			Context.OrganData.Contains(NonVolumeOrgan))
	});
}

N_TEST_HIGH(FNAssemblyOperationContextTests_AddOrganComponent_DuplicateAddIsIdempotent,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContext::AddOrganComponent::DuplicateAddIsIdempotent",
	N_TEST_CONTEXT_EDITOR)
{
	// Re-adding the same organ returns success but must not create a second OrganData/InputComponents entry.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContextAddOrganHarness;
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNOrganComponent* Organ = SpawnVolumeOrgan(World, false);
		if (Organ == nullptr)
		{
			ADD_ERROR("Could not spawn the test organ.");
			return;
		}

		FNAssemblyOperationContext Context(1);
		const bool bFirst = Context.AddOrganComponent(Organ);
		const bool bSecond = Context.AddOrganComponent(Organ);

		CHECK_MESSAGE(TEXT("Both add calls must report success."), bFirst && bSecond)
		CHECK_EQUALS("A duplicate add must not create a second OrganData entry.", Context.OrganData.Num(), 1)
		CHECK_EQUALS("A duplicate add must not create a second InputComponents entry.", Context.InputComponents.Num(), 1)
	});
}

N_TEST_HIGH(FNAssemblyOperationContextTests_AddOrganComponent_UnboundOrganIntersectsEverythingWithInfiniteBounds,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContext::AddOrganComponent::UnboundOrganIntersectsEverythingWithInfiniteBounds",
	N_TEST_CONTEXT_EDITOR)
{
	// An unbound organ cannot be spatially clipped, so it is recorded as intersecting every other organ in the level
	// and given effectively infinite bounds (and never reports anything as "contained").
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContextAddOrganHarness;
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNOrganComponent* Unbound = SpawnVolumeOrgan(World, true);
		UNOrganComponent* OtherA = SpawnVolumeOrgan(World, false);
		UNOrganComponent* OtherB = SpawnVolumeOrgan(World, false);
		if (Unbound == nullptr || OtherA == nullptr || OtherB == nullptr)
		{
			ADD_ERROR("Could not spawn the test organs.");
			return;
		}

		FNAssemblyOperationContext Context(1);
		Context.AddOrganComponent(Unbound);

		const FNWorldOrganData& Data = Context.OrganData[Unbound];
		CHECK_MESSAGE(TEXT("An unbound organ must record every other organ as an intersection."),
			Data.IntersectComponents.Contains(OtherA) && Data.IntersectComponents.Contains(OtherB))
		CHECK_EQUALS("An unbound organ contains nothing (it has no finite volume to enclose with).",
			Data.ContainedComponents.Num(), 0)
		CHECK_MESSAGE(TEXT("An unbound organ must be given effectively infinite bounds."),
			Data.Bounds.BoxExtent.X > 1.0e300)
	});
}

N_TEST_HIGH(FNAssemblyOperationContextTests_AddOrganComponent_AddAfterLockIsRejected,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContext::AddOrganComponent::AddAfterLockIsRejected",
	N_TEST_CONTEXT_EDITOR)
{
	// Once preprocessing has locked the context, further inputs must be rejected so the computed ordering stays valid.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyOperationContextAddOrganHarness;
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNOrganComponent* First = SpawnVolumeOrgan(World, false);
		if (First == nullptr)
		{
			ADD_ERROR("Could not spawn the test organ.");
			return;
		}

		FNAssemblyOperationContext Context(1);
		Context.AddOrganComponent(First);
		Context.LockAndPreprocess(World);

		UNOrganComponent* Second = SpawnVolumeOrgan(World, false);

		// AddOrganComponent logs a warning when it rejects input on a locked context; declare it expected so the
		// deliberate rejection does not surface as unexpected test-log noise.
		AddExpectedMessage(TEXT("has already been locked"), ELogVerbosity::Warning);
		const bool bAdded = Context.AddOrganComponent(Second);

		CHECK_FALSE_MESSAGE(TEXT("Adding an organ after the context is locked must be rejected."), bAdded)
		CHECK_FALSE_MESSAGE(TEXT("A rejected organ must not be recorded."), Context.OrganData.Contains(Second))
	});
}

#endif //WITH_TESTS

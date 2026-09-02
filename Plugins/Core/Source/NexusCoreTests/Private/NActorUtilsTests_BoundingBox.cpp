// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NActorUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Developer/NTestUtils.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Macros/NTestMacros.h"

/**
 * These cover the component-level filtering FNActorUtils::GetFilteredComponentsBoundingBox adds on top of
 * AActor::GetComponentsBoundingBox.
 *
 * What separates the filtered measurement from the actor tag it complements is that the rest of the actor has to go
 * on contributing: the case it exists for is a container actor a generator writes many primitives onto, where
 * excluding the actor would take the whole generated result with it. So the assertions are all shaped the same way —
 * one component parked far from the others, and the question of whether the measured box still reaches it.
 */

namespace NEXUS::UnitTests::NCore::FNActorUtilsBoundingBoxHarness
{
	/** Tag these tests exclude on. Nothing in a throwaway world carries it unless a test puts it there. */
	static const FName IgnoreTag = FName(TEXT("FNActorUtilsTest_Ignore"));

	/** Where the second primitive is parked — far enough out that its contribution to the bounds is unmistakable. */
	constexpr double FarOffset = 10000.0;
}

N_TEST_HIGH(FNActorUtilsTests_BoundingBox_TaggedComponentExcluded,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetFilteredComponentsBoundingBox::TaggedComponentExcluded",
	N_TEST_CONTEXT_EDITOR)
{
	using namespace NEXUS::UnitTests::NCore::FNActorUtilsBoundingBoxHarness;

	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
		if (Cube == nullptr)
		{
			ADD_ERROR("Failed to load /Engine/BasicShapes/Cube");
			return;
		}

		AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>();
		if (MeshActor == nullptr)
		{
			ADD_ERROR("Failed to spawn AStaticMeshActor");
			return;
		}

		UStaticMeshComponent* Kept = MeshActor->GetStaticMeshComponent();
		Kept->SetMobility(EComponentMobility::Movable);
		Kept->SetStaticMesh(Cube);

		UStaticMeshComponent* Ignored = NewObject<UStaticMeshComponent>(MeshActor);
		Ignored->SetupAttachment(MeshActor->GetRootComponent());
		Ignored->RegisterComponent();
		Ignored->SetMobility(EComponentMobility::Movable);
		Ignored->SetStaticMesh(Cube);
		Ignored->SetRelativeLocation(FVector(FarOffset, 0.0, 0.0));
		Ignored->ComponentTags.Add(IgnoreTag);

		const FVector FarPoint = MeshActor->GetActorLocation() + FVector(FarOffset, 0.0, 0.0);

		const FBox Both = FNActorUtils::GetFilteredComponentsBoundingBox(MeshActor, true, TArray<FName>(), false);
		REQUIRE_MESSAGE(TEXT("Measuring with no ignore tags must produce a valid box."), Both.IsValid != 0);
		CHECK_MESSAGE(TEXT("With no ignore tags the box must reach the parked component."), Both.IsInsideOrOn(FarPoint));

		const FBox Filtered = FNActorUtils::GetFilteredComponentsBoundingBox(MeshActor, true, { IgnoreTag }, false);
		CHECK_MESSAGE(TEXT("An untagged sibling must still be measured when one component carries the ignore tag."),
			Filtered.IsValid != 0);
		CHECK_FALSE_MESSAGE(TEXT("The box must no longer reach the tagged component."), Filtered.IsInsideOrOn(FarPoint));
		CHECK_MESSAGE(TEXT("Excluding the tagged component must produce a strictly smaller box."),
			Filtered.GetVolume() < Both.GetVolume());
	});
}

N_TEST_HIGH(FNActorUtilsTests_BoundingBox_NoIgnoreTagsMeasuresEveryComponent,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetFilteredComponentsBoundingBox::NoIgnoreTagsMeasuresEveryComponent",
	N_TEST_CONTEXT_EDITOR)
{
	using namespace NEXUS::UnitTests::NCore::FNActorUtilsBoundingBoxHarness;

	// Every existing caller passes an empty list, and each has to keep measuring exactly what it did. A tagged
	// component is only special to a caller that asked about that tag.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
		if (Cube == nullptr)
		{
			ADD_ERROR("Failed to load /Engine/BasicShapes/Cube");
			return;
		}

		AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>();
		if (MeshActor == nullptr)
		{
			ADD_ERROR("Failed to spawn AStaticMeshActor");
			return;
		}

		UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->SetStaticMesh(Cube);
		MeshComponent->ComponentTags.Add(IgnoreTag);

		const FBox Filtered = FNActorUtils::GetFilteredComponentsBoundingBox(MeshActor, true, TArray<FName>(), false);
		const FBox Engine = MeshActor->GetComponentsBoundingBox(true);

		CHECK_MESSAGE(TEXT("A tagged component must still be measured when no ignore tags were supplied."),
			Filtered.IsValid != 0);
		CHECK_MESSAGE(TEXT("With no ignore tags the measurement must match AActor::GetComponentsBoundingBox."),
			Filtered.Equals(Engine));
	});
}

N_TEST_MEDIUM(FNActorUtilsTests_BoundingBox_EveryComponentTaggedYieldsInvalidBox,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetFilteredComponentsBoundingBox::EveryComponentTaggedYieldsInvalidBox",
	N_TEST_CONTEXT_EDITOR)
{
	using namespace NEXUS::UnitTests::NCore::FNActorUtilsBoundingBoxHarness;

	// An actor made entirely of ignored primitives is still walked, and has to report an invalid box rather than a
	// point at the origin — a caller folding that in as valid would drag its result back to wherever the origin is.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
		if (Cube == nullptr)
		{
			ADD_ERROR("Failed to load /Engine/BasicShapes/Cube");
			return;
		}

		AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>();
		if (MeshActor == nullptr)
		{
			ADD_ERROR("Failed to spawn AStaticMeshActor");
			return;
		}

		UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->SetStaticMesh(Cube);
		MeshComponent->ComponentTags.Add(IgnoreTag);

		const FBox Filtered = FNActorUtils::GetFilteredComponentsBoundingBox(MeshActor, true, { IgnoreTag }, false);
		CHECK_FALSE_MESSAGE(TEXT("An actor whose every primitive is tagged out yields an invalid box."),
			Filtered.IsValid != 0);
	});
}

N_TEST_MEDIUM(FNActorUtilsTests_BoundingBox_BuiltVariantMatchesUnfiltered,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetFilteredComponentsBoundingBox::BuiltVariantMatchesUnfiltered",
	N_TEST_CONTEXT_EDITOR)
{
	using namespace NEXUS::UnitTests::NCore::FNActorUtilsBoundingBoxHarness;

	// GetBuiltComponentsBoundingBox is now a forward onto the filtered walk. This pins the two arguments it supplies,
	// so a later edit cannot quietly hand it an ignore list or drop the placeholder rejection it exists for.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
		if (Cube == nullptr)
		{
			ADD_ERROR("Failed to load /Engine/BasicShapes/Cube");
			return;
		}

		AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>();
		if (MeshActor == nullptr)
		{
			ADD_ERROR("Failed to spawn AStaticMeshActor");
			return;
		}

		UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->SetStaticMesh(Cube);
		MeshComponent->ComponentTags.Add(IgnoreTag);

		const FBox Built = FNActorUtils::GetBuiltComponentsBoundingBox(MeshActor, true);
		const FBox Filtered = FNActorUtils::GetFilteredComponentsBoundingBox(MeshActor, true, TArray<FName>(), true);

		REQUIRE_MESSAGE(TEXT("A cube primitive has built geometry, so both must produce a valid box."), Built.IsValid != 0);
		CHECK_MESSAGE(TEXT("GetBuiltComponentsBoundingBox must measure the unfiltered, placeholder-rejecting walk."),
			Built.Equals(Filtered));
	});
}

N_TEST_MEDIUM(FNActorUtilsTests_BoundingBox_NullActor,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetFilteredComponentsBoundingBox::NullActor",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NCore::FNActorUtilsBoundingBoxHarness;

	// It walks a level's actor array, which routinely holds nulls.
	const FBox Box = FNActorUtils::GetFilteredComponentsBoundingBox(nullptr, true, { IgnoreTag }, false);
	CHECK_FALSE_MESSAGE(TEXT("A null actor yields an invalid box rather than a point at the origin."), Box.IsValid != 0);

	CHECK_FALSE_MESSAGE(TEXT("A null component carries no tags."),
		FNActorUtils::HasAnyComponentTag(nullptr, { IgnoreTag }));
}

#endif //WITH_TESTS

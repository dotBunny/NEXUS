// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblySettings.h"
#include "NWorldCollisionFingerprint.h"
#include "Developer/NTestUtils.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionFingerprintHarness
{
	/** Collision settings with every gather-affecting field set away from its default. */
	static FNWorldAssemblyWorldCollisionSettings MakeBaseline()
	{
		FNWorldAssemblyWorldCollisionSettings Settings;
		Settings.bExcludeNonCollisionEnabledActors = false;
		Settings.bIncludePlayerStarts = false;
		Settings.bIncludeLandscapes = true;
		Settings.bIncludeMeshTerrains = true;
		Settings.LandscapeSampleSpacing = 250.0f;
		Settings.ActorIgnoreTags = { FName(TEXT("Alpha")), FName(TEXT("Beta")) };
		Settings.ComponentIgnoreTags = { FName(TEXT("Delta")), FName(TEXT("Epsilon")) };
		return Settings;
	}
}

N_TEST_CRITICAL(FNWorldCollisionFingerprintTests_HashSettings_EveryGatherFieldChangesIt,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionFingerprint::HashSettings::EveryGatherFieldChangesIt",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionFingerprintHarness;

	// A field that governs what the gather emits but does not reach the hash would let a cache built under one
	// setting be accepted under another — the quietest way this system could serve wrong geometry.
	const uint64 Baseline = FNWorldCollisionFingerprint::HashSettings(MakeBaseline());

	FNWorldAssemblyWorldCollisionSettings Changed = MakeBaseline();
	Changed.bExcludeNonCollisionEnabledActors = true;
	CHECK_MESSAGE(TEXT("bExcludeNonCollisionEnabledActors must reach the hash."),
		FNWorldCollisionFingerprint::HashSettings(Changed) != Baseline);

	Changed = MakeBaseline();
	Changed.bIncludePlayerStarts = true;
	CHECK_MESSAGE(TEXT("bIncludePlayerStarts must reach the hash."),
		FNWorldCollisionFingerprint::HashSettings(Changed) != Baseline);

	Changed = MakeBaseline();
	Changed.bIncludeLandscapes = false;
	CHECK_MESSAGE(TEXT("bIncludeLandscapes must reach the hash."),
		FNWorldCollisionFingerprint::HashSettings(Changed) != Baseline);

	Changed = MakeBaseline();
	Changed.bIncludeMeshTerrains = false;
	CHECK_MESSAGE(TEXT("bIncludeMeshTerrains must reach the hash."),
		FNWorldCollisionFingerprint::HashSettings(Changed) != Baseline);

	Changed = MakeBaseline();
	Changed.LandscapeSampleSpacing = 100.0f;
	CHECK_MESSAGE(TEXT("LandscapeSampleSpacing must reach the hash."),
		FNWorldCollisionFingerprint::HashSettings(Changed) != Baseline);

	Changed = MakeBaseline();
	Changed.ActorIgnoreTags.Add(FName(TEXT("Gamma")));
	CHECK_MESSAGE(TEXT("An added ignore tag must reach the hash."),
		FNWorldCollisionFingerprint::HashSettings(Changed) != Baseline);

	Changed = MakeBaseline();
	Changed.ActorIgnoreTags = { FName(TEXT("Alpha")) };
	CHECK_MESSAGE(TEXT("A removed ignore tag must reach the hash."),
		FNWorldCollisionFingerprint::HashSettings(Changed) != Baseline);

	Changed = MakeBaseline();
	Changed.ComponentIgnoreTags.Add(FName(TEXT("Zeta")));
	CHECK_MESSAGE(TEXT("An added component ignore tag must reach the hash."),
		FNWorldCollisionFingerprint::HashSettings(Changed) != Baseline);

	Changed = MakeBaseline();
	Changed.ComponentIgnoreTags = { FName(TEXT("Delta")) };
	CHECK_MESSAGE(TEXT("A removed component ignore tag must reach the hash."),
		FNWorldCollisionFingerprint::HashSettings(Changed) != Baseline);

	// The two lists are separate filters and must not collapse into one another: moving a tag from the actor list to
	// the component list changes which things it excludes, so it has to change the hash.
	Changed = MakeBaseline();
	Changed.ActorIgnoreTags = { FName(TEXT("Alpha")), FName(TEXT("Beta")), FName(TEXT("Delta")), FName(TEXT("Epsilon")) };
	Changed.ComponentIgnoreTags.Reset();
	CHECK_MESSAGE(TEXT("Folding the component ignore tags into the actor list must change the hash."),
		FNWorldCollisionFingerprint::HashSettings(Changed) != Baseline);
}

N_TEST_HIGH(FNWorldCollisionFingerprintTests_HashSettings_IgnoresIgnoreTagOrder,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionFingerprint::HashSettings::IgnoresIgnoreTagOrder",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionFingerprintHarness;

	// The same set of tags entered in a different order describes the same filter. Re-ordering an array in the
	// details panel must not invalidate every cache in the level.
	FNWorldAssemblyWorldCollisionSettings Reordered = MakeBaseline();
	Reordered.ActorIgnoreTags = { FName(TEXT("Beta")), FName(TEXT("Alpha")) };
	Reordered.ComponentIgnoreTags = { FName(TEXT("Epsilon")), FName(TEXT("Delta")) };

	CHECK_MESSAGE(TEXT("Ignore-tag order must not affect the settings hash."),
		FNWorldCollisionFingerprint::HashSettings(Reordered) ==
		FNWorldCollisionFingerprint::HashSettings(MakeBaseline()));
}

N_TEST_HIGH(FNWorldCollisionFingerprintTests_HashSettings_IsDeterministic,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionFingerprint::HashSettings::IsDeterministic",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionFingerprintHarness;

	CHECK_MESSAGE(TEXT("Identical settings must hash identically."),
		FNWorldCollisionFingerprint::HashSettings(MakeBaseline()) ==
		FNWorldCollisionFingerprint::HashSettings(MakeBaseline()));
}

N_TEST_HIGH(FNWorldCollisionFingerprintTests_HashBounds_IgnoresOrderButNotContent,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionFingerprint::HashBounds::IgnoresOrderButNotContent",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FBoxSphereBounds A(FVector(0.0, 0.0, 0.0), FVector(100.0, 100.0, 100.0), 173.0);
	const FBoxSphereBounds B(FVector(500.0, 0.0, 0.0), FVector(50.0, 50.0, 50.0), 86.0);

	// Order-independent: the bounds array is assembled by walking the operation's generation order, which is not
	// something a stored cache should depend on.
	CHECK_MESSAGE(TEXT("Bounds order must not affect the hash."),
		FNWorldCollisionFingerprint::HashBounds({ A, B }) == FNWorldCollisionFingerprint::HashBounds({ B, A }));

	// Content-sensitive: resizing an organ changes which actors a fresh gather would read, so its cache must not
	// survive the resize.
	const FBoxSphereBounds Grown(FVector(0.0, 0.0, 0.0), FVector(200.0, 100.0, 100.0), 245.0);
	CHECK_MESSAGE(TEXT("Resizing a bound must change the hash."),
		FNWorldCollisionFingerprint::HashBounds({ A, B }) != FNWorldCollisionFingerprint::HashBounds({ Grown, B }));

	const FBoxSphereBounds Moved(FVector(10.0, 0.0, 0.0), FVector(100.0, 100.0, 100.0), 173.0);
	CHECK_MESSAGE(TEXT("Moving a bound must change the hash."),
		FNWorldCollisionFingerprint::HashBounds({ A, B }) != FNWorldCollisionFingerprint::HashBounds({ Moved, B }));

	CHECK_MESSAGE(TEXT("Dropping a bound must change the hash."),
		FNWorldCollisionFingerprint::HashBounds({ A, B }) != FNWorldCollisionFingerprint::HashBounds({ A }));
}

N_TEST_HIGH(FNWorldCollisionFingerprintTests_OverlapsBounds_EmptyBoundsAcceptEverything,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionFingerprint::OverlapsBounds::EmptyBoundsAcceptEverything",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Mirrors the raw-mesh factory, where an empty bounds array means "whole level" rather than "nothing". An
	// unbounded organ empties the array, so getting this backwards would cache an empty world for it.
	CHECK_MESSAGE(TEXT("Empty bounds must accept even a null actor's test, matching the factory's skip-the-filter path."),
		FNWorldCollisionFingerprint::OverlapsBounds(nullptr, {}));
}

N_TEST_CRITICAL(FNWorldCollisionFingerprintTests_HashActor_ComponentTagsChangeIt,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionFingerprint::HashActor::ComponentTagsChangeIt",
	N_TEST_CONTEXT_EDITOR)
{
	// The pairing that keeps component-level exclusion honest. A component tag decides whether the gather emits that
	// component's geometry, so the fingerprint guarding the cache has to move when the tag does — otherwise tagging a
	// component changes what a bake would produce while the stored cache goes on reporting itself current, and the
	// geometry the author just excluded stays in the pool. Silent, and the only failure here that serves wrong
	// geometry rather than merely re-baking too often.
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
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		const uint64 Untagged = FNWorldCollisionFingerprint::HashActor(MeshActor);

		MeshComponent->ComponentTags.Add(NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore);
		const uint64 Tagged = FNWorldCollisionFingerprint::HashActor(MeshActor);

		CHECK_MESSAGE(TEXT("Tagging a component must change the actor's fingerprint."), Tagged != Untagged);

		MeshComponent->ComponentTags.RemoveSwap(NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore);
		CHECK_MESSAGE(TEXT("Removing the tag again must return the actor's fingerprint to what it was."),
			FNWorldCollisionFingerprint::HashActor(MeshActor) == Untagged);

		// Order-independent, matching the ignore lists: whatever wrote a component's tags is free to write them in a
		// different order, and that must not invalidate a level's caches.
		MeshComponent->ComponentTags = { FName(TEXT("Alpha")), FName(TEXT("Beta")) };
		const uint64 OneOrder = FNWorldCollisionFingerprint::HashActor(MeshActor);
		MeshComponent->ComponentTags = { FName(TEXT("Beta")), FName(TEXT("Alpha")) };
		CHECK_MESSAGE(TEXT("Re-ordering a component's tags must not change the fingerprint."),
			FNWorldCollisionFingerprint::HashActor(MeshActor) == OneOrder);
	});
}

#endif //WITH_TESTS

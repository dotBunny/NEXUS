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

N_TEST_CRITICAL(FNWorldCollisionFingerprintTests_HashActor_ExclusionChangesItAndNothingElseDoes,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionFingerprint::HashActor::ExclusionChangesItAndNothingElseDoes",
	N_TEST_CONTEXT_EDITOR)
{
	// Two halves that have to hold together, and holding only the first is what caused a real bug.
	//
	// A component tagged out of the gather must move the fingerprint, or the cache would go on reporting itself
	// current while holding geometry that is no longer emitted. But *only* the exclusion may move it: hashing the
	// whole tag list also worked for the first half, and made the fingerprint sensitive to every tag anything
	// writes. Generators rewrite component tags on each generate — PCG stamps its spawned components with its own
	// marker and the source component's name — so a level holding generated content hashed differently every
	// session and its cache could never validate, however untouched the level was.
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

		const TArray<FName> IgnoreTags = { NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore };

		const uint64 Untagged = FNWorldCollisionFingerprint::HashActor(MeshActor, IgnoreTags);

		// Tags nothing is looking for must be invisible. This is the half the bug broke.
		MeshComponent->ComponentTags.Add(FName(TEXT("PCG.GeneratedThisSession_0")));
		CHECK_MESSAGE(TEXT("A tag outside the ignore list must not move the fingerprint."),
			FNWorldCollisionFingerprint::HashActor(MeshActor, IgnoreTags) == Untagged);

		MeshComponent->ComponentTags.Add(FName(TEXT("PCG.GeneratedThisSession_1")));
		CHECK_MESSAGE(TEXT("Nor must a second one, however many accumulate."),
			FNWorldCollisionFingerprint::HashActor(MeshActor, IgnoreTags) == Untagged);

		// The exclusion itself must still register, or the cache could outlive the geometry it describes.
		MeshComponent->ComponentTags.Add(NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore);
		const uint64 Excluded = FNWorldCollisionFingerprint::HashActor(MeshActor, IgnoreTags);
		CHECK_MESSAGE(TEXT("Tagging a component out of the gather must move the fingerprint."),
			Excluded != Untagged);

		MeshComponent->ComponentTags.RemoveSwap(NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore);
		CHECK_MESSAGE(TEXT("Untagging it again must return the fingerprint to what it was."),
			FNWorldCollisionFingerprint::HashActor(MeshActor, IgnoreTags) == Untagged);

		// And an empty ignore list excludes nothing, so the same component reads as it did before it was tagged.
		MeshComponent->ComponentTags.Add(NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore);
		CHECK_MESSAGE(TEXT("With nothing on the ignore list, the ignore tag is just another tag."),
			FNWorldCollisionFingerprint::HashActor(MeshActor, {}) ==
			FNWorldCollisionFingerprint::HashActor(MeshActor, { FName(TEXT("SomethingElse")) }));

		// Once excluded, the component's placement stops being the cache's business — the gather emits nothing for
		// it, so a cache guarded against where it sits would be guarding against something it does not contain.
		// This is most of the point of tagging a component out: a generator that rewrites its output on every load
		// otherwise goes on invalidating the level's cache forever, however little the level itself changed.
		const uint64 ExcludedBeforeMove = FNWorldCollisionFingerprint::HashActor(MeshActor, IgnoreTags);
		MeshActor->SetActorLocation(FVector(1234.0, 567.0, 89.0));
		CHECK_MESSAGE(TEXT("Moving an excluded component must not move the fingerprint."),
			FNWorldCollisionFingerprint::HashActor(MeshActor, IgnoreTags) == ExcludedBeforeMove);

		// The same move must register the moment the component is back in the gather, or the exclusion would be
		// buying its silence by making the fingerprint permanently deaf to that component.
		MeshComponent->ComponentTags.RemoveSwap(NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore);
		const uint64 IncludedAfterMove = FNWorldCollisionFingerprint::HashActor(MeshActor, IgnoreTags);
		MeshActor->SetActorLocation(FVector::ZeroVector);
		CHECK_MESSAGE(TEXT("Moving an included component must still move the fingerprint."),
			FNWorldCollisionFingerprint::HashActor(MeshActor, IgnoreTags) != IncludedAfterMove);
	});
}
#endif //WITH_TESTS

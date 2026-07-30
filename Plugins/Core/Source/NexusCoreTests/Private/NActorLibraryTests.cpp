// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NActorLibrary.h"
#include "Developer/NDebugActor.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(UNActorLibraryTests_IsSameActors_BothEmpty,
	"NEXUS::UnitTests::NCore::UNActorLibrary::IsSameActors::BothEmpty",
	N_TEST_CONTEXT_ANYWHERE)
{
	const TArray<AActor*> A;
	const TArray<AActor*> B;
	CHECK_MESSAGE(TEXT("Two empty arrays must compare equal"), UNActorLibrary::IsSameActors(A, B));
}

N_TEST_HIGH(UNActorLibraryTests_IsSameActors_DifferentLength,
	"NEXUS::UnitTests::NCore::UNActorLibrary::IsSameActors::DifferentLength",
	N_TEST_CONTEXT_ANYWHERE)
{
	const TArray<AActor*> A = {nullptr, nullptr};
	const TArray<AActor*> B = {nullptr};
	CHECK_FALSE_MESSAGE(TEXT("Arrays of different sizes must short-circuit to false"), UNActorLibrary::IsSameActors(A, B));
}

N_TEST_HIGH(UNActorLibraryTests_IsSameActors_Equal_SameOrder,
	"NEXUS::UnitTests::NCore::UNActorLibrary::IsSameActors::Equal_SameOrder",
	N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		ANDebugActor* X = World->SpawnActor<ANDebugActor>();
		ANDebugActor* Y = World->SpawnActor<ANDebugActor>();
		if (X == nullptr || Y == nullptr)
		{
			ADD_ERROR("Failed to spawn debug actors for the test world");
			return;
		}

		const TArray<AActor*> A = {X, Y};
		const TArray<AActor*> B = {X, Y};
		CHECK_MESSAGE(TEXT("Identical arrays in same order must compare equal"), UNActorLibrary::IsSameActors(A, B));
	});
}

N_TEST_HIGH(UNActorLibraryTests_IsSameActors_Equal_Reordered,
	"NEXUS::UnitTests::NCore::UNActorLibrary::IsSameActors::Equal_Reordered",
	N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		ANDebugActor* X = World->SpawnActor<ANDebugActor>();
		ANDebugActor* Y = World->SpawnActor<ANDebugActor>();
		ANDebugActor* Z = World->SpawnActor<ANDebugActor>();
		if (X == nullptr || Y == nullptr || Z == nullptr)
		{
			ADD_ERROR("Failed to spawn debug actors for the test world");
			return;
		}

		const TArray<AActor*> A = {X, Y, Z};
		const TArray<AActor*> B = {Z, X, Y};
		CHECK_MESSAGE(TEXT("Same actors in different order must compare equal"), UNActorLibrary::IsSameActors(A, B));
	});
}

N_TEST_HIGH(UNActorLibraryTests_IsSameActors_Disjoint,
	"NEXUS::UnitTests::NCore::UNActorLibrary::IsSameActors::Disjoint",
	N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		ANDebugActor* X = World->SpawnActor<ANDebugActor>();
		ANDebugActor* Y = World->SpawnActor<ANDebugActor>();
		ANDebugActor* Z = World->SpawnActor<ANDebugActor>();
		ANDebugActor* W = World->SpawnActor<ANDebugActor>();
		if (X == nullptr || Y == nullptr || Z == nullptr || W == nullptr)
		{
			ADD_ERROR("Failed to spawn debug actors for the test world");
			return;
		}

		const TArray<AActor*> A = {X, Y};
		const TArray<AActor*> B = {Z, W};
		CHECK_FALSE_MESSAGE(TEXT("Disjoint arrays of equal length must compare unequal"), UNActorLibrary::IsSameActors(A, B));
	});
}

N_TEST_CRITICAL(UNActorLibraryTests_IsSameActors_Duplicates_Match,
	"NEXUS::UnitTests::NCore::UNActorLibrary::IsSameActors::Duplicates_Match",
	N_TEST_CONTEXT_EDITOR)
{
	// Regression: [X, X] vs [X, X] previously returned false because the inner loop consumed every
	// match for A[0] in B, leaving A[1] with nothing to claim. Multiset comparison must respect counts.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		ANDebugActor* X = World->SpawnActor<ANDebugActor>();
		if (X == nullptr)
		{
			ADD_ERROR("Failed to spawn debug actor for the test world");
			return;
		}

		const TArray<AActor*> A = {X, X};
		const TArray<AActor*> B = {X, X};
		CHECK_MESSAGE(TEXT("[X, X] vs [X, X] must compare equal"), UNActorLibrary::IsSameActors(A, B));
	});
}

N_TEST_HIGH(UNActorLibraryTests_IsSameActors_Duplicates_Reordered,
	"NEXUS::UnitTests::NCore::UNActorLibrary::IsSameActors::Duplicates_Reordered",
	N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		ANDebugActor* X = World->SpawnActor<ANDebugActor>();
		ANDebugActor* Y = World->SpawnActor<ANDebugActor>();
		if (X == nullptr || Y == nullptr)
		{
			ADD_ERROR("Failed to spawn debug actors for the test world");
			return;
		}

		const TArray<AActor*> A = {X, X, Y};
		const TArray<AActor*> B = {X, Y, X};
		CHECK_MESSAGE(TEXT("Same multiset in different order must compare equal"), UNActorLibrary::IsSameActors(A, B));
	});
}

N_TEST_HIGH(UNActorLibraryTests_IsSameActors_Duplicates_CountMismatch,
	"NEXUS::UnitTests::NCore::UNActorLibrary::IsSameActors::Duplicates_CountMismatch",
	N_TEST_CONTEXT_EDITOR)
{
	// Same distinct elements but different multiplicities — set equality would pass, multiset equality must not.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		ANDebugActor* X = World->SpawnActor<ANDebugActor>();
		ANDebugActor* Y = World->SpawnActor<ANDebugActor>();
		if (X == nullptr || Y == nullptr)
		{
			ADD_ERROR("Failed to spawn debug actors for the test world");
			return;
		}

		const TArray<AActor*> A = {X, X, Y};
		const TArray<AActor*> B = {X, Y, Y};
		CHECK_FALSE_MESSAGE(TEXT("Different multiplicities must compare unequal"), UNActorLibrary::IsSameActors(A, B));
	});
}

N_TEST_MEDIUM(UNActorLibraryTests_IsSameActors_Nullptr_Match,
	"NEXUS::UnitTests::NCore::UNActorLibrary::IsSameActors::Nullptr_Match",
	N_TEST_CONTEXT_EDITOR)
{
	// Nullptr is a valid AActor* value the caller could pass — equal counts on both sides must compare equal.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		ANDebugActor* X = World->SpawnActor<ANDebugActor>();
		if (X == nullptr)
		{
			ADD_ERROR("Failed to spawn debug actor for the test world");
			return;
		}

		const TArray<AActor*> A = {X, nullptr};
		const TArray<AActor*> B = {nullptr, X};
		CHECK_MESSAGE(TEXT("Nullptr entries must participate equally in the multiset comparison"), UNActorLibrary::IsSameActors(A, B));
	});
}

#endif //WITH_TESTS

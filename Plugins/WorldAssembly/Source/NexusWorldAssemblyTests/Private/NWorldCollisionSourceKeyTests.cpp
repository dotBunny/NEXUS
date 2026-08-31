// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldCollisionSourceKey.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNWorldCollisionSourceKeyTests_Mix_IsDeterministicAndSpreads,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionSourceKey::Mix::IsDeterministicAndSpreads",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The same input always mixes to the same value, or no cache could ever be validated across runs.
	CHECK_MESSAGE(TEXT("Mix must be deterministic."),
		FNWorldCollisionSourceKey::Mix(1234) == FNWorldCollisionSourceKey::Mix(1234));

	// Adjacent small integers are the common case here (instance indices, element ordinals). They must not land in
	// adjacent buckets, or a combined key would be dominated by whichever component happened to be largest.
	const uint64 A = FNWorldCollisionSourceKey::Mix(0);
	const uint64 B = FNWorldCollisionSourceKey::Mix(1);
	CHECK_MESSAGE(TEXT("Adjacent inputs must not mix to adjacent values."),
		A != B && (A > B ? A - B : B - A) > 1024);
}

N_TEST_HIGH(FNWorldCollisionSourceKeyTests_Combine_IsOrderDependent,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionSourceKey::Combine::IsOrderDependent",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Combine composes an ordered identity (level, actor, component, instance, ordinal). Swapping two of those
	// describes a different element, so it must not collide.
	CHECK_MESSAGE(TEXT("Combine must not be commutative."),
		FNWorldCollisionSourceKey::Combine(7, 9) != FNWorldCollisionSourceKey::Combine(9, 7));
}

N_TEST_HIGH(FNWorldCollisionSourceKeyTests_HashString_IsStableAndDistinguishes,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionSourceKey::HashString::IsStableAndDistinguishes",
	N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("Equal strings must hash equal."),
		FNWorldCollisionSourceKey::HashString(TEXT("StaticMeshActor_0")) ==
		FNWorldCollisionSourceKey::HashString(TEXT("StaticMeshActor_0")));

	CHECK_MESSAGE(TEXT("Strings differing by one character must hash apart."),
		FNWorldCollisionSourceKey::HashString(TEXT("StaticMeshActor_0")) !=
		FNWorldCollisionSourceKey::HashString(TEXT("StaticMeshActor_1")));
}

N_TEST_HIGH(FNWorldCollisionSourceKeyTests_HashName_MatchesItsText,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionSourceKey::HashName::MatchesItsText",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Hashed by text rather than comparison index: the index is assigned per session, so a cache baked in the editor
	// would never validate in a packaged build if identity rode on it.
	const FName Name(TEXT("SM_Wall_01"));
	CHECK_MESSAGE(TEXT("An FName must hash as its own text does."),
		FNWorldCollisionSourceKey::HashName(Name) == FNWorldCollisionSourceKey::HashString(TEXT("SM_Wall_01")));
}

N_TEST_HIGH(FNWorldCollisionSourceKeyTests_HashGuid_DistinguishesEveryComponent,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionSourceKey::HashGuid::DistinguishesEveryComponent",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FGuid Baseline(1, 2, 3, 4);
	CHECK_MESSAGE(TEXT("An identical GUID must hash equal."),
		FNWorldCollisionSourceKey::HashGuid(Baseline) == FNWorldCollisionSourceKey::HashGuid(FGuid(1, 2, 3, 4)));

	// Each component carries body-setup identity; ignoring any one of them would let a swapped mesh pass validation.
	CHECK_MESSAGE(TEXT("A differing A component must change the hash."),
		FNWorldCollisionSourceKey::HashGuid(Baseline) != FNWorldCollisionSourceKey::HashGuid(FGuid(9, 2, 3, 4)));
	CHECK_MESSAGE(TEXT("A differing B component must change the hash."),
		FNWorldCollisionSourceKey::HashGuid(Baseline) != FNWorldCollisionSourceKey::HashGuid(FGuid(1, 9, 3, 4)));
	CHECK_MESSAGE(TEXT("A differing C component must change the hash."),
		FNWorldCollisionSourceKey::HashGuid(Baseline) != FNWorldCollisionSourceKey::HashGuid(FGuid(1, 2, 9, 4)));
	CHECK_MESSAGE(TEXT("A differing D component must change the hash."),
		FNWorldCollisionSourceKey::HashGuid(Baseline) != FNWorldCollisionSourceKey::HashGuid(FGuid(1, 2, 3, 9)));
}

N_TEST_HIGH(FNWorldCollisionSourceKeyTests_HashTransform_IgnoresSubQuantumNoise,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionSourceKey::HashTransform::IgnoresSubQuantumNoise",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Quantization exists so that recomputing a transform through slightly different arithmetic does not read as a
	// moved actor and throw away an otherwise good cache.
	const FTransform Baseline(FQuat::Identity, FVector(100.0, 200.0, 300.0), FVector::OneVector);
	const FTransform Nudged(FQuat::Identity, FVector(100.0 + 1e-6, 200.0, 300.0), FVector::OneVector);

	CHECK_MESSAGE(TEXT("Sub-quantum movement must not change the hash."),
		FNWorldCollisionSourceKey::HashTransform(Baseline) == FNWorldCollisionSourceKey::HashTransform(Nudged));
}

N_TEST_CRITICAL(FNWorldCollisionSourceKeyTests_HashTransform_DetectsRealMovement,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionSourceKey::HashTransform::DetectsRealMovement",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The inverse of the test above, and the one that actually protects correctness: a moved actor must invalidate.
	const FTransform Baseline(FQuat::Identity, FVector(100.0, 200.0, 300.0), FVector::OneVector);

	CHECK_MESSAGE(TEXT("A one-unit translation must change the hash."),
		FNWorldCollisionSourceKey::HashTransform(Baseline) !=
		FNWorldCollisionSourceKey::HashTransform(FTransform(FQuat::Identity, FVector(101.0, 200.0, 300.0), FVector::OneVector)));

	CHECK_MESSAGE(TEXT("A rotation must change the hash."),
		FNWorldCollisionSourceKey::HashTransform(Baseline) !=
		FNWorldCollisionSourceKey::HashTransform(FTransform(FRotator(0.0, 90.0, 0.0).Quaternion(), FVector(100.0, 200.0, 300.0), FVector::OneVector)));

	CHECK_MESSAGE(TEXT("A scale change must change the hash."),
		FNWorldCollisionSourceKey::HashTransform(Baseline) !=
		FNWorldCollisionSourceKey::HashTransform(FTransform(FQuat::Identity, FVector(100.0, 200.0, 300.0), FVector(2.0, 1.0, 1.0))));
}

N_TEST_HIGH(FNWorldCollisionSourceKeyTests_HashTransform_IgnoresQuaternionSign,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionSourceKey::HashTransform::IgnoresQuaternionSign",
	N_TEST_CONTEXT_ANYWHERE)
{
	// q and -q are the same orientation. Engine math reaches one or the other depending on how a rotation was
	// composed, so hashing them apart would invalidate caches for actors nobody touched.
	const FQuat Rotation = FRotator(15.0, 45.0, 30.0).Quaternion();
	const FQuat Negated(-Rotation.X, -Rotation.Y, -Rotation.Z, -Rotation.W);

	const FTransform A(Rotation, FVector::ZeroVector, FVector::OneVector);
	const FTransform B(Negated, FVector::ZeroVector, FVector::OneVector);

	CHECK_MESSAGE(TEXT("A negated quaternion describes the same orientation and must hash the same."),
		FNWorldCollisionSourceKey::HashTransform(A) == FNWorldCollisionSourceKey::HashTransform(B));
}

#endif //WITH_TESTS

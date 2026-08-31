// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Macros/NTestMacros.h"
#include "Organ/NOrganCollisionCache.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNOrganCollisionCacheTests_HasData_IsFalseUntilBaked,
	"NEXUS::UnitTests::NWorldAssembly::FNOrganCollisionCache::HasData::IsFalseUntilBaked",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNOrganCollisionCache Cache;
	CHECK_FALSE_MESSAGE(TEXT("A default cache must report no data."), Cache.HasData());
	CHECK_FALSE_MESSAGE(TEXT("A default cache must never validate, not even against a zero fingerprint."),
		Cache.IsValidFor(0));
}

N_TEST_CRITICAL(FNOrganCollisionCacheTests_IsValidFor_MatchesOnlyItsOwnFingerprint,
	"NEXUS::UnitTests::NWorldAssembly::FNOrganCollisionCache::IsValidFor::MatchesOnlyItsOwnFingerprint",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The entire validity test. A cache that accepted a fingerprint other than its own would hand an assembly
	// geometry describing a level that no longer exists.
	FNOrganCollisionCache Cache;
	Cache.SetBakeResult(0x1234ABCDULL, { 1, 2, 3 });

	CHECK_MESSAGE(TEXT("A cache must validate against the fingerprint it was baked with."), Cache.IsValidFor(0x1234ABCDULL));
	CHECK_FALSE_MESSAGE(TEXT("A cache must not validate against a different fingerprint."), Cache.IsValidFor(0x1234ABCEULL));
	CHECK_FALSE_MESSAGE(TEXT("A cache must not validate against the never-baked sentinel."), Cache.IsValidFor(0));
}

N_TEST_HIGH(FNOrganCollisionCacheTests_SetBakeResult_SortsAndDeduplicates,
	"NEXUS::UnitTests::NWorldAssembly::FNOrganCollisionCache::SetBakeResult::SortsAndDeduplicates",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Sorted so a re-bake of unchanged geometry is byte-identical and stays out of the next changelist; de-duplicated
	// so an element named twice is not resolved and collision-tested twice.
	FNOrganCollisionCache Cache;
	Cache.SetBakeResult(1, { 30, 10, 20, 10, 30 });

	CHECK_EQUALS("Duplicate keys must collapse.", Cache.SourceKeys.Num(), 3);
	CHECK_MESSAGE(TEXT("Keys must be stored in ascending order."),
		Cache.SourceKeys[0] == 10 && Cache.SourceKeys[1] == 20 && Cache.SourceKeys[2] == 30);
}

N_TEST_HIGH(FNOrganCollisionCacheTests_SetBakeResult_IsOrderIndependent,
	"NEXUS::UnitTests::NWorldAssembly::FNOrganCollisionCache::SetBakeResult::IsOrderIndependent",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two bakes that found the same geometry in a different world-iteration order must produce the same stored state.
	FNOrganCollisionCache First;
	First.SetBakeResult(1, { 5, 1, 9 });

	FNOrganCollisionCache Second;
	Second.SetBakeResult(1, { 9, 5, 1 });

	CHECK_MESSAGE(TEXT("Gather order must not affect the stored key list."), First.SourceKeys == Second.SourceKeys);
}

N_TEST_HIGH(FNOrganCollisionCacheTests_Reset_ReturnsToNeverBaked,
	"NEXUS::UnitTests::NWorldAssembly::FNOrganCollisionCache::Reset::ReturnsToNeverBaked",
	N_TEST_CONTEXT_ANYWHERE)
{
	FNOrganCollisionCache Cache;
	Cache.SetBakeResult(0x99ULL, { 1, 2 });
	Cache.Reset();

	CHECK_FALSE_MESSAGE(TEXT("A reset cache must report no data."), Cache.HasData());
	CHECK_EQUALS("A reset cache must hold no keys.", Cache.SourceKeys.Num(), 0);
}

N_TEST_CRITICAL(FNOrganCollisionCacheTests_Identical_DiffersFromADefaultCache,
	"NEXUS::UnitTests::NWorldAssembly::FNOrganCollisionCache::Identical::DiffersFromADefaultCache",
	N_TEST_CONTEXT_ANYWHERE)
{
	// This is what decides whether the cache is written to disk at all. UE skips saving a property it considers equal
	// to its default, and none of this struct's state is reflected — so without a hand-written Identical it compares
	// an empty property list, calls every cache default, and a baked organ silently loads back unbaked.
	const FNOrganCollisionCache Default;

	FNOrganCollisionCache Baked;
	Baked.SetBakeResult(0x1234ABCDULL, { 1, 2, 3 });

	CHECK_FALSE_MESSAGE(TEXT("A baked cache must not compare identical to a default one, or it is never saved."),
		Baked.Identical(&Default, 0));
	CHECK_MESSAGE(TEXT("A cache must compare identical to a copy of itself."), Baked.Identical(&Baked, 0));
	CHECK_FALSE_MESSAGE(TEXT("Null is never identical."), Baked.Identical(nullptr, 0));
}

N_TEST_HIGH(FNOrganCollisionCacheTests_Identical_DetectsEachStoredField,
	"NEXUS::UnitTests::NWorldAssembly::FNOrganCollisionCache::Identical::DetectsEachStoredField",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A field this misses is a field whose change never reaches disk.
	FNOrganCollisionCache Baseline;
	Baseline.SetBakeResult(0x1234ABCDULL, { 1, 2, 3 });

	FNOrganCollisionCache DifferentFingerprint = Baseline;
	DifferentFingerprint.Fingerprint = 0x9999ULL;
	CHECK_FALSE_MESSAGE(TEXT("A differing fingerprint must not compare identical."),
		Baseline.Identical(&DifferentFingerprint, 0));

	FNOrganCollisionCache DifferentKeys = Baseline;
	DifferentKeys.SourceKeys.Add(99);
	CHECK_FALSE_MESSAGE(TEXT("A differing key list must not compare identical."),
		Baseline.Identical(&DifferentKeys, 0));
}

N_TEST_CRITICAL(FNOrganCollisionCacheTests_Serialize_RoundTrips,
	"NEXUS::UnitTests::NWorldAssembly::FNOrganCollisionCache::Serialize::RoundTrips",
	N_TEST_CONTEXT_ANYWHERE)
{
	FNOrganCollisionCache Written;
	Written.SetBakeResult(0xFEEDFACEDEADBEEFULL, { 300, 100, 200 });

	TArray<uint8> Bytes;
	FMemoryWriter Writer(Bytes);
	Written.Serialize(Writer);

	FNOrganCollisionCache Read;
	FMemoryReader Reader(Bytes);
	Read.Serialize(Reader);

	CHECK_MESSAGE(TEXT("The fingerprint must survive the round trip."), Read.Fingerprint == 0xFEEDFACEDEADBEEFULL);
	CHECK_MESSAGE(TEXT("The key list must survive the round trip intact and ordered."),
		Read.SourceKeys == Written.SourceKeys);
	CHECK_MESSAGE(TEXT("A loaded cache must still validate against its own fingerprint."),
		Read.IsValidFor(0xFEEDFACEDEADBEEFULL));
	CHECK_MESSAGE(TEXT("The bake time must survive the round trip."), Read.BakeTime == Written.BakeTime);
}

#endif //WITH_TESTS

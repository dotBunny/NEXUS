// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NMersenneTwister.h"
#include "Math/NMersenneTwisterFriendlyState.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(FNMersenneTwisterTests_SaveRestore_ReproducesSequenceAfterIntegerDraws,
	"NEXUS::UnitTests::NCore::FNMersenneTwister::SaveRestore_ReproducesSequenceAfterIntegerDraws",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the whole point of the snapshot: it reproduces the exact sequence even after
	// rejection-sampled integer draws, which a naive seed + logical-call-count replay cannot.
	constexpr uint64 Seed = 0xC0FFEEull;
	FNMersenneTwister Twister(Seed);

	// Advance with a deliberate mix that exercises the rejection-sampling integer path.
	for (int32 i = 0; i < 37; ++i)
	{
		Twister.Float();
		Twister.IntegerRange(0, 1000);
		Twister.Bool();
	}

	const FNMersenneTwisterState State = Twister.SaveState();

	// Capture the "truth" sequence that follows the snapshot.
	TArray<float> Expected;
	Expected.Reserve(50);
	for (int32 i = 0; i < 50; ++i)
	{
		Expected.Add(Twister.Float());
	}

	// Restore into a DIFFERENTLY seeded twister and confirm it reproduces the post-snapshot sequence.
	FNMersenneTwister Restored(1ull);
	CHECK_MESSAGE(TEXT("RestoreState should succeed for a valid snapshot."), Restored.RestoreState(State));
	CHECK_MESSAGE(TEXT("RestoreState should restore the originating seed."), Restored.GetInitialSeed() == Seed);
	CHECK_MESSAGE(TEXT("RestoreState should restore the draw count."), Restored.GetCallCounter() == State.DrawCount);

	for (int32 i = 0; i < 50; ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Restored Float[%d] should match the original post-snapshot sequence."), i),
			FMath::IsNearlyEqual(Restored.Float(), Expected[i]));
	}
}

N_TEST_HIGH(FNMersenneTwisterTests_SaveRestore_DrawCountMatchesEngineAdvances,
	"NEXUS::UnitTests::NCore::FNMersenneTwister::SaveRestore_DrawCountMatchesEngineAdvances",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies CallCounter is now an exact engine-draw count: replaying that many draws from the seed
	// reproduces the next raw value. This guards the rejection-sampling (undercount) and Max<=Min
	// early-out (overcount) paths that previously desynced the counter.
	constexpr uint64 Seed = 777ull;
	FNMersenneTwister A(Seed);
	for (int32 i = 0; i < 25; ++i)
	{
		A.IntegerRange(0, 7);          // rejection-sampling path (variable draws per call)
		A.UnsignedIntegerRange(5, 5);  // Max<=Min early-out path (zero draws)
		A.Double();
	}

	const uint64 Draws = A.GetCallCounter();
	const uint64 NextFromA = A.UnsignedInteger64();

	// Independently replay the raw engine the recorded number of draws; the next raw value must match.
	FNMersenneTwister B(0ull);
	CHECK_MESSAGE(TEXT("RestoreState should accept the recorded draw count."),
		B.RestoreState(FNMersenneTwisterState{ Seed, Draws }));
	const uint64 NextFromB = B.UnsignedInteger64();

	CHECK_MESSAGE(TEXT("CallCounter must be an exact engine draw count (replaying it reproduces the next raw value)."),
		NextFromA == NextFromB);
}

N_TEST_MEDIUM(FNMersenneTwisterTests_SaveRestore_TokenRoundTrip,
	"NEXUS::UnitTests::NCore::FNMersenneTwister::SaveRestore_TokenRoundTrip",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNMersenneTwisterState Original{ 0xDEADBEEFull, 123456ull };
	const FString Token = Original.ToString();
	const FNMersenneTwisterState Parsed = FNMersenneTwisterState::FromString(Token);

	CHECK_MESSAGE(TEXT("ToString/FromString should round-trip InitialSeed."), Parsed.InitialSeed == Original.InitialSeed);
	CHECK_MESSAGE(TEXT("ToString/FromString should round-trip DrawCount."), Parsed.DrawCount == Original.DrawCount);
}

N_TEST_MEDIUM(FNMersenneTwisterTests_SaveRestore_FriendlyStateRoundTrip,
	"NEXUS::UnitTests::NCore::FNMersenneTwister::SaveRestore_FriendlyStateRoundTrip",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNMersenneTwisterState Native{ 0x12345678ull, 999ull };
	const FNMersenneTwisterFriendlyState Friendly = FNMersenneTwisterFriendlyState::FromNative(Native);
	const FNMersenneTwisterState Back = Friendly.ToNative();

	CHECK_MESSAGE(TEXT("Friendly conversion should preserve InitialSeed."), Back.InitialSeed == Native.InitialSeed);
	CHECK_MESSAGE(TEXT("Friendly conversion should preserve DrawCount."), Back.DrawCount == Native.DrawCount);
	CHECK_FALSE_MESSAGE(TEXT("Friendly InitialSeed should be a non-empty presentable string."), Friendly.InitialSeed.IsEmpty());
}

N_TEST_MEDIUM(FNMersenneTwisterTests_SaveRestore_RejectsOversizedDrawCount,
	"NEXUS::UnitTests::NCore::FNMersenneTwister::SaveRestore_RejectsOversizedDrawCount",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies a corrupt/hand-edited draw count is refused rather than fed to a pathological discard,
	// and that a refused restore leaves the live twister untouched.
	constexpr uint64 Seed = 42ull;
	FNMersenneTwister Twister(Seed);
	for (int32 i = 0; i < 10; ++i)
	{
		Twister.Float();
	}
	const uint64 BeforeSeed = Twister.GetInitialSeed();
	const uint64 BeforeCounter = Twister.GetCallCounter();

	// RestoreState intentionally logs a warning when it refuses the snapshot; expect it so CI does
	// not treat the captured log as a failure.
	AddExpectedMessage(TEXT("RestoreState refused a snapshot"), ELogVerbosity::Warning);

	const FNMersenneTwisterState Bad{ Seed, FNMersenneTwister::MaxRestoreDrawCount + 1 };
	CHECK_FALSE_MESSAGE(TEXT("RestoreState should refuse a draw count above MaxRestoreDrawCount."), Twister.RestoreState(Bad));
	CHECK_MESSAGE(TEXT("A refused RestoreState must leave the seed unchanged."), Twister.GetInitialSeed() == BeforeSeed);
	CHECK_MESSAGE(TEXT("A refused RestoreState must leave the draw count unchanged."), Twister.GetCallCounter() == BeforeCounter);
}

#endif //WITH_TESTS

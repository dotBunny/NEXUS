// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NMersenneTwister.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(FNMersenneTwisterTests_Determinism_SameSeedSameSequence, "NEXUS::UnitTests::NCore::FNMersenneTwister::Determinism_SameSeedSameSequence", N_TEST_CONTEXT_ANYWHERE)
{
	constexpr uint64 Seed = 12345678ull;
	FNMersenneTwister TwisterA(Seed);
	FNMersenneTwister TwisterB(Seed);

	for (int32 i = 0; i < 100; ++i)
	{
		const float A = TwisterA.Float();
		const float B = TwisterB.Float();
		CHECK_MESSAGE(FString::Printf(TEXT("Float[%d] from identical seeds should be equal"), i), FMath::IsNearlyEqual(A, B));
	}
}

N_TEST_HIGH(FNMersenneTwisterTests_Determinism_DifferentSeedsDifferentSequence, "NEXUS::UnitTests::NCore::FNMersenneTwister::Determinism_DifferentSeedsDifferentSequence", N_TEST_CONTEXT_ANYWHERE)
{
	FNMersenneTwister TwisterA(11111ull);
	FNMersenneTwister TwisterB(99999ull);

	bool bFoundDifference = false;
	for (int32 i = 0; i < 20; ++i)
	{
		if (!FMath::IsNearlyEqual(TwisterA.Float(), TwisterB.Float()))
		{
			bFoundDifference = true;
			break;
		}
	}
	CHECK_MESSAGE(TEXT("Different seeds should produce different sequences"), bFoundDifference);
}

N_TEST_HIGH(FNMersenneTwisterTests_Reset_RestoresInitialState, "NEXUS::UnitTests::NCore::FNMersenneTwister::Reset_RestoresInitialState", N_TEST_CONTEXT_ANYWHERE)
{
	constexpr uint64 Seed = 42ull;
	FNMersenneTwister Twister(Seed);

	// Capture first sequence
	TArray<float> FirstRun;
	FirstRun.Reserve(50);
	for (int32 i = 0; i < 50; ++i)
	{
		FirstRun.Add(Twister.Float());
	}

	Twister.Reset();

	// Capture second sequence after reset
	for (int32 i = 0; i < 50; ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Float[%d] after Reset() should match first run"), i), FMath::IsNearlyEqual(Twister.Float(), FirstRun[i]));
	}
}

N_TEST_HIGH(FNMersenneTwisterTests_GetInitialSeed, "NEXUS::UnitTests::NCore::FNMersenneTwister::GetInitialSeed", N_TEST_CONTEXT_ANYWHERE)
{
	constexpr uint64 Seed = 987654321ull;
	FNMersenneTwister Twister(Seed);

	// Exhaust some values to make sure the seed is persisted across calls
	for (int32 i = 0; i < 200; ++i) { Twister.Float(); }

	CHECK_MESSAGE(TEXT("GetInitialSeed should return the original seed after many calls"), Twister.GetInitialSeed() == Seed);
}

N_TEST_HIGH(FNMersenneTwisterTests_Float_Range, "NEXUS::UnitTests::NCore::FNMersenneTwister::Float_Range", N_TEST_CONTEXT_ANYWHERE)
{
	FNMersenneTwister Twister(100ull);
	for (int32 i = 0; i < 1000; ++i)
	{
		const float Value = Twister.Float();
		CHECK_MESSAGE(FString::Printf(TEXT("Float()[%d] should be >= 0"), i), Value >= 0.f);
		CHECK_MESSAGE(FString::Printf(TEXT("Float()[%d] should be <= 1"), i), Value <= 1.f);
	}
}

N_TEST_HIGH(FNMersenneTwisterTests_Double_Range, "NEXUS::UnitTests::NCore::FNMersenneTwister::Double_Range", N_TEST_CONTEXT_ANYWHERE)
{
	FNMersenneTwister Twister(200ull);
	for (int32 i = 0; i < 1000; ++i)
	{
		const double Value = Twister.Double();
		CHECK_MESSAGE(FString::Printf(TEXT("Double()[%d] should be >= 0"), i), Value >= 0.0);
		CHECK_MESSAGE(FString::Printf(TEXT("Double()[%d] should be <= 1"), i), Value <= 1.0);
	}
}

N_TEST_HIGH(FNMersenneTwisterTests_FloatRange_Bounds, "NEXUS::UnitTests::NCore::FNMersenneTwister::FloatRange_Bounds", N_TEST_CONTEXT_ANYWHERE)
{
	FNMersenneTwister Twister(300ull);
	constexpr float Min = -5.f;
	constexpr float Max = 5.f;
	for (int32 i = 0; i < 500; ++i)
	{
		const float Value = Twister.FloatRange(Min, Max);
		CHECK_MESSAGE(FString::Printf(TEXT("FloatRange[%d] should be >= Min"), i), Value >= Min);
		CHECK_MESSAGE(FString::Printf(TEXT("FloatRange[%d] should be <= Max"), i), Value <= Max);
	}
}

N_TEST_HIGH(FNMersenneTwisterTests_IntegerRange_Bounds, "NEXUS::UnitTests::NCore::FNMersenneTwister::IntegerRange_Bounds", N_TEST_CONTEXT_ANYWHERE)
{
	FNMersenneTwister Twister(400ull);
	constexpr int32 Min = -10;
	constexpr int32 Max = 10;
	for (int32 i = 0; i < 500; ++i)
	{
		const int32 Value = Twister.IntegerRange(Min, Max);
		CHECK_MESSAGE(FString::Printf(TEXT("IntegerRange[%d] should be >= Min"), i), Value >= Min);
		CHECK_MESSAGE(FString::Printf(TEXT("IntegerRange[%d] should be <= Max"), i), Value <= Max);
	}
}

N_TEST_HIGH(FNMersenneTwisterTests_Bias_ExtremeValues, "NEXUS::UnitTests::NCore::FNMersenneTwister::Bias_ExtremeValues", N_TEST_CONTEXT_ANYWHERE)
{
	FNMersenneTwister Twister(500ull);

	// 0% chance should always return false
	for (int32 i = 0; i < 20; ++i)
	{
		CHECK_FALSE_MESSAGE(FString::Printf(TEXT("Bias(0.0)[%d] should always be false"), i), Twister.Bias(0.f));
	}

	// 100% chance should always return true
	for (int32 i = 0; i < 20; ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Bias(1.0)[%d] should always be true"), i), Twister.Bias(1.f));
	}
}

N_TEST_HIGH(FNMersenneTwisterTests_CallCounter_Increments, "NEXUS::UnitTests::NCore::FNMersenneTwister::CallCounter_Increments", N_TEST_CONTEXT_ANYWHERE)
{
	FNMersenneTwister Twister(600ull);
	CHECK_MESSAGE(TEXT("Call counter should start at 0"), Twister.GetCallCounter() == 0u);

	Twister.Float();
	CHECK_MESSAGE(TEXT("Call counter should be 1 after one Float() call"), Twister.GetCallCounter() == 1u);

	Twister.Float();
	Twister.Float();
	CHECK_MESSAGE(TEXT("Call counter should be 3 after three Float() calls"), Twister.GetCallCounter() == 3u);

	Twister.Reset();
	CHECK_MESSAGE(TEXT("Call counter should reset to 0 after Reset()"), Twister.GetCallCounter() == 0u);
}

N_TEST_HIGH(FNMersenneTwisterTests_ArrayBulk_Float, "NEXUS::UnitTests::NCore::FNMersenneTwister::ArrayBulk_Float", N_TEST_CONTEXT_ANYWHERE)
{
	FNMersenneTwister Twister(700ull);
	constexpr int32 Count = 100;

	TArray<float> Output;
	Output.SetNum(Count);
	Twister.Float(Output, Count);

	for (int32 i = 0; i < Count; ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Bulk float[%d] should be in [0,1]"), i), Output[i] >= 0.f && Output[i] <= 1.f);
	}
}

N_TEST_HIGH(FNMersenneTwisterTests_ArrayBulk_IntegerRange, "NEXUS::UnitTests::NCore::FNMersenneTwister::ArrayBulk_IntegerRange", N_TEST_CONTEXT_ANYWHERE)
{
	FNMersenneTwister Twister(800ull);
	constexpr int32 Count = 100;
	constexpr int32 Min = 0;
	constexpr int32 Max = 255;

	TArray<int32> Output;
	Output.SetNum(Count);
	Twister.IntegerRange(Output, Count, Min, Max);

	for (int32 i = 0; i < Count; ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Bulk integer[%d] should be in [0,255]"), i), Output[i] >= Min && Output[i] <= Max);
	}
}

#endif //WITH_TESTS

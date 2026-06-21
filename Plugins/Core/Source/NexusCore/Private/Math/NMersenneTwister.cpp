// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NMersenneTwister.h"
#include "NCoreMinimal.h"

uint64 FNMersenneTwister::NextBounded(const uint64 Range)
{
	// Callers guarantee Range >= 1. Rejection sampling removes the modulo bias that a plain
	// (Engine() % Range) would introduce, while keeping the result a pure function of the
	// engine's portable output (no STL distribution involved).
	const uint64 Threshold = (0ULL - Range) % Range; // == 2^64 mod Range
	uint64 Value;
	do
	{
		Value = this->Draw();
	}
	while (Value < Threshold);
	return Value % Range;
}

bool FNMersenneTwister::Bias(const float Chance)
{
	return this->Float() < Chance;
}

void FNMersenneTwister::Bias(TArray<bool>& OutArray, const int32 Count, const float Chance, const int32 StartIndex)
{
	const int32 ActualCount = StartIndex + Count;
	checkf(OutArray.Num() >= ActualCount,
		TEXT("OutArray (%d) is smaller than StartIndex + Count (%d)."), OutArray.Num(), ActualCount);
	for (int32 i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = ToUnitFloat(this->Draw()) < Chance;
	}
}

bool FNMersenneTwister::Bool()
{
	// Use the most-significant bit of a single engine draw as a portable, unbiased coin flip.
	return (this->Draw() >> 63) != 0ULL;
}

void FNMersenneTwister::Bool(TArray<bool>& OutArray, const int32 Count, const int32 StartIndex)
{
	const int32 ActualCount = StartIndex + Count;
	checkf(OutArray.Num() >= ActualCount,
		TEXT("OutArray (%d) is smaller than StartIndex + Count (%d)."), OutArray.Num(), ActualCount);
	for (int32 i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = (this->Draw() >> 63) != 0ULL;
	}
}

double FNMersenneTwister::Double()
{
	return ToUnitDouble(this->Draw());
}

void FNMersenneTwister::Double(TArray<double>& OutArray, const int32 Count, const int32 StartIndex)
{
	const int32 ActualCount = StartIndex + Count;
	checkf(OutArray.Num() >= ActualCount,
		TEXT("OutArray (%d) is smaller than StartIndex + Count (%d)."), OutArray.Num(), ActualCount);
	for (int32 i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = ToUnitDouble(this->Draw());
	}
}

void FNMersenneTwister::DoubleRange(TArray<double>& OutArray, const int32 Count, const double MinimumValue,
	const double MaximumValue, const int32 StartIndex)
{
	const int32 ActualCount = StartIndex + Count;
	checkf(OutArray.Num() >= ActualCount,
		TEXT("OutArray (%d) is smaller than StartIndex + Count (%d)."), OutArray.Num(), ActualCount);
	const double Span = MaximumValue - MinimumValue;
	for (int32 i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = MinimumValue + ToUnitDouble(this->Draw()) * Span;
	}
}

float FNMersenneTwister::Float()
{
	return ToUnitFloat(this->Draw());
}

void FNMersenneTwister::Float(TArray<float>& OutArray, const int32 Count, const int32 StartIndex)
{
	const int32 ActualCount = StartIndex + Count;
	checkf(OutArray.Num() >= ActualCount,
		TEXT("OutArray (%d) is smaller than StartIndex + Count (%d)."), OutArray.Num(), ActualCount);
	for (int32 i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = ToUnitFloat(this->Draw());
	}
}

float FNMersenneTwister::FloatRange(const float MinimumValue, const float MaximumValue)
{
	return MinimumValue + ToUnitFloat(this->Draw()) * (MaximumValue - MinimumValue);
}

double FNMersenneTwister::DoubleRange(const double MinimumValue, const double MaximumValue)
{
	return MinimumValue + ToUnitDouble(this->Draw()) * (MaximumValue - MinimumValue);
}

void FNMersenneTwister::FloatRange(TArray<float>& OutArray, const int32 Count, const float MinimumValue,
	const float MaximumValue, const int32 StartIndex)
{
	const int32 ActualCount = StartIndex + Count;
	checkf(OutArray.Num() >= ActualCount,
		TEXT("OutArray (%d) is smaller than StartIndex + Count (%d)."), OutArray.Num(), ActualCount);
	const float Span = MaximumValue - MinimumValue;
	for (int32 i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = MinimumValue + ToUnitFloat(this->Draw()) * Span;
	}
}

int32 FNMersenneTwister::IntegerRange(const int32 MinimumValue, const int32 MaximumValue)
{
	if (MaximumValue <= MinimumValue)
	{
		return MinimumValue;
	}
	// Inclusive [Min, Max]; widen to int64 so the span never overflows for full-range inputs.
	const uint64 Span = static_cast<uint64>(static_cast<int64>(MaximumValue) - static_cast<int64>(MinimumValue) + 1);
	return static_cast<int32>(static_cast<int64>(MinimumValue) + static_cast<int64>(NextBounded(Span)));
}

void FNMersenneTwister::IntegerRange(TArray<int32>& OutArray, const int32 Count, const int32 MinimumValue,
	const int32 MaximumValue, const int32 StartIndex)
{
	const int32 ActualCount = StartIndex + Count;
	checkf(OutArray.Num() >= ActualCount,
		TEXT("OutArray (%d) is smaller than StartIndex + Count (%d)."), OutArray.Num(), ActualCount);
	if (MaximumValue <= MinimumValue)
	{
		for (int32 i = StartIndex; i < ActualCount; i++)
		{
			OutArray[i] = MinimumValue;
		}
		return;
	}
	const uint64 Span = static_cast<uint64>(static_cast<int64>(MaximumValue) - static_cast<int64>(MinimumValue) + 1);
	for (int32 i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = static_cast<int32>(static_cast<int64>(MinimumValue) + static_cast<int64>(NextBounded(Span)));
	}
}

uint32 FNMersenneTwister::UnsignedIntegerRange(const uint32 MinimumValue, const uint32 MaximumValue)
{
	if (MaximumValue <= MinimumValue)
	{
		return MinimumValue;
	}
	// Inclusive [Min, Max]; span computed in uint64 so the full uint32 range cannot overflow.
	const uint64 Span = static_cast<uint64>(MaximumValue) - static_cast<uint64>(MinimumValue) + 1;
	return static_cast<uint32>(static_cast<uint64>(MinimumValue) + NextBounded(Span));
}

void FNMersenneTwister::UnsignedIntegerRange(TArray<uint32>& OutArray, const int32 Count, const uint32 MinimumValue,
	const uint32 MaximumValue, const int32 StartIndex)
{
	const int32 ActualCount = StartIndex + Count;
	checkf(OutArray.Num() >= ActualCount,
		TEXT("OutArray (%d) is smaller than StartIndex + Count (%d)."), OutArray.Num(), ActualCount);
	if (MaximumValue <= MinimumValue)
	{
		for (int32 i = StartIndex; i < ActualCount; i++)
		{
			OutArray[i] = MinimumValue;
		}
		return;
	}
	const uint64 Span = static_cast<uint64>(MaximumValue) - static_cast<uint64>(MinimumValue) + 1;
	for (int32 i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = static_cast<uint32>(static_cast<uint64>(MinimumValue) + NextBounded(Span));
	}
}

FVector FNMersenneTwister::Vector(const float MinimumRange, const float MaximumRange)
{
	const float Span = MaximumRange - MinimumRange;
	const FVector PseudoRandomValue = FVector(
		MinimumRange + ToUnitFloat(this->Draw()) * Span,
		MinimumRange + ToUnitFloat(this->Draw()) * Span,
		MinimumRange + ToUnitFloat(this->Draw()) * Span);
	return PseudoRandomValue;
}

void FNMersenneTwister::Initialize(const uint64 Seed)
{
	this->InitialSeed = Seed;
	this->Engine.seed(Seed);
	UE_LOG(LogNexusCore, VeryVerbose, TEXT("Initialized FNMersenneTwister(%llu)."), Seed);
	this->CallCounter = 0;
}

bool FNMersenneTwister::RestoreState(const FNMersenneTwisterState& State)
{
	if (State.DrawCount > MaxRestoreDrawCount)
	{
		UE_LOG(LogNexusCore, Warning,
			TEXT("FNMersenneTwister::RestoreState refused a snapshot: DrawCount (%llu) exceeds MaxRestoreDrawCount (%llu); likely corrupt data. The engine was left unchanged."),
			State.DrawCount, MaxRestoreDrawCount);
		return false;
	}

	// Re-seed then replay forward. discard() is defined as N calls to the engine, and both the engine
	// and discard() are fully standard-specified, so this reproduces the exact state on any
	// platform/compiler. Initialize() resets CallCounter to 0, so restore it to the replayed count.
	this->Initialize(State.InitialSeed);
	this->Engine.discard(State.DrawCount);
	this->CallCounter = State.DrawCount;
	return true;
}

bool FNMersenneTwisterState::IsValid() const
{
	return DrawCount <= FNMersenneTwister::MaxRestoreDrawCount;
}

FString FNMersenneTwisterState::ToString() const
{
	return FString::Printf(TEXT("%s-%s"),
		*FNSeedGenerator::HexFromSeed(InitialSeed),
		*FNSeedGenerator::HexFromSeed(DrawCount));
}

FNMersenneTwisterState FNMersenneTwisterState::FromString(const FString& InState)
{
	FString SeedPart;
	FString DrawPart;
	if (!InState.Split(TEXT("-"), &SeedPart, &DrawPart))
	{
		UE_LOG(LogNexusCore, Warning,
			TEXT("FNMersenneTwisterState::FromString could not parse '%s'; expected '<SeedHex>-<DrawCountHex>'. Returning a zeroed state."),
			*InState);
		return FNMersenneTwisterState{};
	}
	return FNMersenneTwisterState{
		FNSeedGenerator::SeedFromHex(SeedPart),
		FNSeedGenerator::SeedFromHex(DrawPart) };
}

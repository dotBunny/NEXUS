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
		Value = this->Engine();
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
		OutArray[i] = ToUnitFloat(this->Engine()) < Chance;
	}
	this->CallCounter += Count;
}

bool FNMersenneTwister::Bool()
{
	// Use the most-significant bit of a single engine draw as a portable, unbiased coin flip.
	const bool bPseudoRandomValue = (this->Engine() >> 63) != 0ULL;
	this->CallCounter++;
	return bPseudoRandomValue;
}

void FNMersenneTwister::Bool(TArray<bool>& OutArray, const int32 Count, const int32 StartIndex)
{
	const int32 ActualCount = StartIndex + Count;
	checkf(OutArray.Num() >= ActualCount,
		TEXT("OutArray (%d) is smaller than StartIndex + Count (%d)."), OutArray.Num(), ActualCount);
	for (int32 i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = (this->Engine() >> 63) != 0ULL;
	}
	this->CallCounter += Count;
}

double FNMersenneTwister::Double()
{
	const double PseudoRandomValue = ToUnitDouble(this->Engine());
	this->CallCounter++;
	return PseudoRandomValue;
}

void FNMersenneTwister::Double(TArray<double>& OutArray, const int32 Count, const int32 StartIndex)
{
	const int32 ActualCount = StartIndex + Count;
	checkf(OutArray.Num() >= ActualCount,
		TEXT("OutArray (%d) is smaller than StartIndex + Count (%d)."), OutArray.Num(), ActualCount);
	for (int32 i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = ToUnitDouble(this->Engine());
	}
	this->CallCounter += Count;
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
		OutArray[i] = MinimumValue + ToUnitDouble(this->Engine()) * Span;
	}
	this->CallCounter += Count;
}

float FNMersenneTwister::Float()
{
	const float PseudoRandomValue = ToUnitFloat(this->Engine());
	this->CallCounter++;
	return PseudoRandomValue;
}

void FNMersenneTwister::Float(TArray<float>& OutArray, const int32 Count, const int32 StartIndex)
{
	const int32 ActualCount = StartIndex + Count;
	checkf(OutArray.Num() >= ActualCount,
		TEXT("OutArray (%d) is smaller than StartIndex + Count (%d)."), OutArray.Num(), ActualCount);
	for (int32 i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = ToUnitFloat(this->Engine());
	}
	this->CallCounter += Count;
}

float FNMersenneTwister::FloatRange(const float MinimumValue, const float MaximumValue)
{
	const float PseudoRandomValue = MinimumValue + ToUnitFloat(this->Engine()) * (MaximumValue - MinimumValue);
	this->CallCounter++;
	return PseudoRandomValue;
}

double FNMersenneTwister::DoubleRange(const double MinimumValue, const double MaximumValue)
{
	const double PseudoRandomValue = MinimumValue + ToUnitDouble(this->Engine()) * (MaximumValue - MinimumValue);
	this->CallCounter++;
	return PseudoRandomValue;
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
		OutArray[i] = MinimumValue + ToUnitFloat(this->Engine()) * Span;
	}
	this->CallCounter += Count;
}

int32 FNMersenneTwister::IntegerRange(const int32 MinimumValue, const int32 MaximumValue)
{
	this->CallCounter++;
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
		this->CallCounter += Count;
		return;
	}
	const uint64 Span = static_cast<uint64>(static_cast<int64>(MaximumValue) - static_cast<int64>(MinimumValue) + 1);
	for (int32 i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = static_cast<int32>(static_cast<int64>(MinimumValue) + static_cast<int64>(NextBounded(Span)));
	}
	this->CallCounter += Count;
}

uint32 FNMersenneTwister::UnsignedIntegerRange(const uint32 MinimumValue, const uint32 MaximumValue)
{
	this->CallCounter++;
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
		this->CallCounter += Count;
		return;
	}
	const uint64 Span = static_cast<uint64>(MaximumValue) - static_cast<uint64>(MinimumValue) + 1;
	for (int32 i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = static_cast<uint32>(static_cast<uint64>(MinimumValue) + NextBounded(Span));
	}
	this->CallCounter += Count;
}

FVector FNMersenneTwister::Vector(const float MinimumRange, const float MaximumRange)
{
	const float Span = MaximumRange - MinimumRange;
	const FVector PseudoRandomValue = FVector(
		MinimumRange + ToUnitFloat(this->Engine()) * Span,
		MinimumRange + ToUnitFloat(this->Engine()) * Span,
		MinimumRange + ToUnitFloat(this->Engine()) * Span);
	this->CallCounter += 3;
	return PseudoRandomValue;
}

void FNMersenneTwister::Initialize(const uint64 Seed)
{
	this->InitialSeed = Seed;
	this->Engine.seed(Seed);
	UE_LOG(LogNexusCore, VeryVerbose, TEXT("Initialized FNMersenneTwister(%llu)."), Seed);
	this->CallCounter = 0;
}

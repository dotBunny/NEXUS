// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NMersenneTwister.h"
#include "NCoreMinimal.h"

bool FNMersenneTwister::Bias(const float Chance)
{
	// No chance at all
	if (FMath::IsNearlyZero(Chance))
	{
		return false;
	}
	
	if (this->Float() <= Chance)
	{
		return true;
	}

	return false;
}

void FNMersenneTwister::Bias(TArray<bool>& OutArray, const int32 Count, const float Chance, const int32 StartIndex)
{
	const int ActualCount = StartIndex + Count;
	std::uniform_real_distribution<float> Distribution = std::uniform_real_distribution<float>(0.0, 1.0);
	for (int i = StartIndex; i < ActualCount; i++)
	{
		if (Distribution(this->Engine) <= Chance)
		{
			OutArray[i] = true;
		}
		else
		{
			OutArray[i] = false;
		}
	}
	this->CallCounter += Count;
}

bool FNMersenneTwister::Bool()
{
	const bool bPseudoRandomValue = this->BooleanDistribution(this->Engine);
	this->CallCounter++;
	return bPseudoRandomValue;
}

void FNMersenneTwister::Bool(TArray<bool>& OutArray, const int32 Count, const int32 StartIndex)
{
	const int ActualCount = StartIndex + Count;
	for (int i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = this->BooleanDistribution(this->Engine);
	}
	this->CallCounter += Count;
}

double FNMersenneTwister::Double()
{
	const double PseudoRandomValue = this->PersistentDoubleRangeDistribution(this->Engine);
	this->CallCounter++;
	return PseudoRandomValue;
}

void FNMersenneTwister::Double(TArray<double>& OutArray, const int32 Count, const int32 StartIndex)
{
	const int ActualCount = StartIndex + Count;
	for (int i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = this->PersistentDoubleRangeDistribution(this->Engine);
	}
	this->CallCounter += Count;
}

void FNMersenneTwister::DoubleRange(TArray<double>& OutArray, const int32 Count, const double MinimumValue,
	const double MaximumValue, const int32 StartIndex)
{
	const int ActualCount = StartIndex + Count;
	std::uniform_real_distribution<double> Distribution = std::uniform_real_distribution<double>(MinimumValue, MaximumValue);
	for (int i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = Distribution(this->Engine);
	}
	this->CallCounter += Count;
}

float FNMersenneTwister::Float()
{
	const float PseudoRandomValue = this->PersistentFloatRangeDistribution(this->Engine);
	this->CallCounter++;
	return PseudoRandomValue;
}

void FNMersenneTwister::Float(TArray<float>& OutArray, const int32 Count, const int32 StartIndex)
{
	const int ActualCount = StartIndex + Count;
	for (int i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = this->PersistentFloatRangeDistribution(this->Engine);
	}
	this->CallCounter += Count;
}

float FNMersenneTwister::FloatRange(const float MinimumValue, const float MaximumValue)
{
	const float PseudoRandomValue = std::uniform_real_distribution<float>(MinimumValue, MaximumValue)(this->Engine);
	this->CallCounter++;
	return PseudoRandomValue;
}

float FNMersenneTwister::DoubleRange(const double MinimumValue, const double MaximumValue)
{
	const double PseudoRandomValue = std::uniform_real_distribution<double>(MinimumValue, MaximumValue)(this->Engine);
	this->CallCounter++;
	return PseudoRandomValue;
}

void FNMersenneTwister::FloatRange(TArray<float>& OutArray, const int32 Count, const float MinimumValue,
	const float MaximumValue, const int32 StartIndex)
{
	const int ActualCount = StartIndex + Count;
	std::uniform_real_distribution<float> Distribution = std::uniform_real_distribution<float>(MinimumValue, MaximumValue);
	for (int i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = Distribution(this->Engine);
	}
	this->CallCounter += Count;
}

int FNMersenneTwister::IntegerRange(const int MinimumValue, const int MaximumValue)
{
	const int PseudoRandomValue = std::uniform_int_distribution<int>(MinimumValue, MaximumValue)(this->Engine);
	this->CallCounter++;
	return PseudoRandomValue;
}

void FNMersenneTwister::IntegerRange(TArray<int32>& OutArray, const int32 Count, const int32 MinimumValue,
	const int32 MaximumValue, const int32 StartIndex)
{
	const int ActualCount = StartIndex + Count;
	std::uniform_int_distribution<int32> Distribution = std::uniform_int_distribution<int32>(MinimumValue, MaximumValue);
	for (int i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = Distribution(this->Engine);
	}
	this->CallCounter += Count;
}

uint32 FNMersenneTwister::UnsignedIntegerRange(const uint32 MinimumValue, const uint32 MaximumValue)
{
	const uint32 PseudoRandomValue =  std::uniform_int_distribution<uint32>(MinimumValue, MaximumValue)(this->Engine);
	this->CallCounter++;
	return PseudoRandomValue;
}

void FNMersenneTwister::UnsignedIntegerRange(TArray<uint32>& OutArray, const int32 Count, const uint32 MinimumValue,
	const uint32 MaximumValue, const int32 StartIndex)
{
	const int ActualCount = StartIndex + Count;
	std::uniform_int_distribution<uint32> Distribution = std::uniform_int_distribution<uint32>(MinimumValue, MaximumValue);
	for (int i = StartIndex; i < ActualCount; i++)
	{
		OutArray[i] = Distribution(this->Engine);
	}
	this->CallCounter += Count;
}

FVector FNMersenneTwister::VectorNormalized()
{
	const FVector PseudoRandomValue = FVector(
		this->PersistentFloatRangeDistribution(this->Engine),
		this->PersistentFloatRangeDistribution(this->Engine),
		this->PersistentFloatRangeDistribution(this->Engine)
	);
	this->CallCounter += 3;
	return PseudoRandomValue;
}

void FNMersenneTwister::Initialize(const uint64 Seed)
{
	this->InitialSeed = Seed;
	this->Engine.seed(Seed);
	N_LOG(Log, TEXT("[FNMersenneTwister::Initialize] Seed set to %llu"), Seed);
	this->CallCounter = 0;
}

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

	// If it's inclusive we nailed it
	if (this->Float() <= Chance)
	{
		return true;
	}

	return false;
}

bool FNMersenneTwister::Bool()
{
	const bool bPseudoRandomValue = this->BooleanDistribution(this->Engine);
	this->CallCounter++;
	this->BooleanDistribution.reset();

	return bPseudoRandomValue;
}

double FNMersenneTwister::Double()
{
	this->DoubleRangeDistribution = std::uniform_real_distribution<double>(0.0, 1.0);
	const double PseudoRandomValue = this->DoubleRangeDistribution(this->Engine);
	this->CallCounter++;
	
	// TODO: Might not need this if were constantly setting the distribution
	this->DoubleRangeDistribution.reset();

	return PseudoRandomValue;
}

float FNMersenneTwister::Float()
{
	this->FloatRangeDistribution = std::uniform_real_distribution<float>(0.0, 1.0);
	const float PseudoRandomValue = this->FloatRangeDistribution(this->Engine);
	this->CallCounter++;


	// TODO: Might not need this if were constantly setting the distribution
	this->FloatRangeDistribution.reset();

	return PseudoRandomValue;
}

float FNMersenneTwister::FloatRange(const float MinimumValue, const float MaximumValue)
{
	this->FloatRangeDistribution = std::uniform_real_distribution<float>(MinimumValue, MaximumValue);

	const float PseudoRandomValue = this->FloatRangeDistribution(this->Engine);
	this->CallCounter++;

	// TODO: Might not need this if were constantly setting the distribution
	this->FloatRangeDistribution.reset();

	return PseudoRandomValue;
}

float FNMersenneTwister::DoubleRange(const double MinimumValue, const double MaximumValue)
{
	this->DoubleRangeDistribution = std::uniform_real_distribution<double>(MinimumValue, MaximumValue);

	const double PseudoRandomValue = this->DoubleRangeDistribution(this->Engine);
	this->CallCounter++;

	// TODO: Might not need this if were constantly setting the distribution
	this->DoubleRangeDistribution.reset();

	return PseudoRandomValue;
}

int FNMersenneTwister::IntegerRange(const int MinimumValue, const int MaximumValue)
{
	this->IntegerRangeDistribution = std::uniform_int_distribution<int>(MinimumValue, MaximumValue);

	const int PseudoRandomValue = this->IntegerRangeDistribution(this->Engine);
	this->CallCounter++;

	// TODO: Might not need this if were constantly setting the distribution
	this->IntegerRangeDistribution.reset();

	return PseudoRandomValue;
}

uint32 FNMersenneTwister::UnsignedIntegerRange(const uint32 MinimumValue, const uint32 MaximumValue)
{
	this->UnsignedIntegerRangeDistribution = std::uniform_int_distribution<uint32>(MinimumValue, MaximumValue);

	const uint32 PseudoRandomValue = this->UnsignedIntegerRangeDistribution(this->Engine);
	this->CallCounter++;

	// TODO: Might not need this if were constantly setting the distribution
	this->UnsignedIntegerRangeDistribution.reset();

	return PseudoRandomValue;
}

FVector FNMersenneTwister::VectorNormalized()
{
	this->FloatRangeDistribution = std::uniform_real_distribution<float>(0.0, 1.0);
	const FVector PseudoRandomValue = FVector(
		this->FloatRangeDistribution(this->Engine),
		this->FloatRangeDistribution(this->Engine),
		this->FloatRangeDistribution(this->Engine)
	);
	this->CallCounter += 3;


	// TODO: Might not need this if were constantly setting the distribution
	this->FloatRangeDistribution.reset();

	return PseudoRandomValue;
}

void FNMersenneTwister::Initialize(const uint64 Seed)
{
	this->InitialSeed = Seed;
	this->Engine.seed(Seed);
	N_LOG(Log, TEXT("[FNMersenneTwister::Initialize] Seed set to %llu"), Seed);
	this->CallCounter = 0;
}

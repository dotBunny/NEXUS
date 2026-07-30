// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NMersenneTwisterFriendlyState.h"

#include "Math/NMersenneTwister.h"
#include "Math/NSeedGenerator.h"

FNMersenneTwisterState FNMersenneTwisterFriendlyState::ToNative() const
{
	return FNMersenneTwisterState{
		FNSeedGenerator::SeedFromHex(InitialSeed),
		static_cast<uint64>(DrawCount) };
}

FNMersenneTwisterFriendlyState FNMersenneTwisterFriendlyState::FromNative(const FNMersenneTwisterState& State)
{
	FNMersenneTwisterFriendlyState Result;
	Result.InitialSeed = FNSeedGenerator::HexFromSeed(State.InitialSeed);
	Result.DrawCount = static_cast<int64>(State.DrawCount);
	return Result;
}

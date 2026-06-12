// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NMersenneTwister.h"
#include "NRandom.h"
#include "NMersenneTwisterObject.generated.h"

/**
 * A BlueprintType UObject wrapper that owns an FNMersenneTwister, exposing a seedable
 * deterministic random stream to Blueprint with its lifecycle managed by the garbage collector.
 */
UCLASS(BlueprintType, ClassGroup = "NEXUS", DisplayName = "NEXUS | Mersenne Twister Object")
class NEXUSCORE_API UNMersenneTwisterObject : public UObject
{
	GENERATED_BODY()

	/** Seeds the owned twister from the non-deterministic stream so unseeded instances differ per run. */
	UNMersenneTwisterObject()
	{
		Twister = MakeShared<FNMersenneTwister>(FNRandom::GetNonDeterministic().RandHelper(MAX_int32));
	}

	virtual ~UNMersenneTwisterObject() override
	{
		Twister.Reset();
	}
	
public:

	/** Re-seeds the owned twister from a human-friendly seed string. */
	UFUNCTION(BlueprintCallable, DisplayName="Seed", Category = "NEXUS|Core|Random")
	void Seed(const FString Seed) const
	{
		Twister.Get()->Initialize(FNSeedGenerator::SeedFromFriendlySeed(Seed));
	}
	
	/** Returns a pseudo random uniformly distributed bool value. */
	UFUNCTION(BlueprintCallable, DisplayName="Random Bool", Category = "NEXUS|Core|Random")
	bool Bool()
	{
		return Twister->Bool();
	}

	/** Returns an array of Count pseudo random bool values based on a coin-flip. */
	UFUNCTION(BlueprintCallable, DisplayName="Random Bools", Category = "NEXUS|Core|Random")
	TArray<bool> Bools(int32 Count)
	{
		TArray<bool> Result;
		Result.SetNum(Count);
		Twister->Bool(Result, Count, 0);
		return Result;
	}
	
	/** Generates a pseudo random integer between MinimumValue and MaximumValue (inclusive). */
	UFUNCTION(BlueprintCallable, DisplayName="Random Integer In Range", Category = "NEXUS|Core|Random")
	int32 Integer(int32 MinimumValue, int32 MaximumValue)
	{
		return Twister.Get()->IntegerRange(MinimumValue, MaximumValue);
	}
	
	/** Returns a raw pointer to the owned twister. @remark Native code only; do not cache past this object's lifetime. */
	FNMersenneTwister* GetTwister() const
	{
		return Twister.Get();
	}
	
	/** Returns a reference to the owned twister. @remark Native code only; do not cache past this object's lifetime. */
	FNMersenneTwister& GetTwisterRef() const
	{
		return *Twister.Get();
	}
	
private:
	/** The owned twister backing all sampling on this object. */
	TSharedPtr<FNMersenneTwister> Twister;
};
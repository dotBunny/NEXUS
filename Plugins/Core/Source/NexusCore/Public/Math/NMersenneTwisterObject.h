// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NMersenneTwister.h"
#include "NRandom.h"

UCLASS(BlueprintType)
class UNMersenneTwisterObject : public UObject
{
	GENERATED_BODY()
	
	UNMersenneTwisterObject()
	{
		Twister = MakeShared<FNMersenneTwister>(FNRandom::GetNonDeterministic().RandHelper(MAX_int32));
	}

	virtual ~UNMersenneTwisterObject() override
	{
		Twister.Reset();
	}
	
public:
	UFUNCTION(BlueprintCallable)
	void Seed(const FString Seed) const
	{
		Twister.Get()->Initialize(FNSeedGenerator::SeedFromFriendlySeed(Seed));
	}
	
private:
	TSharedPtr<FNMersenneTwister> Twister;
};
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NMersenneTwister.h"
#include "NRandom.h"
#include "NMersenneTwisterObject.generated.h"

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
	
	UFUNCTION(BlueprintCallable)
	bool Bool()
	{
		return Twister->Bool();
	}

	UFUNCTION(BlueprintCallable)
	TArray<bool> Bools(int32 Count)
	{
		TArray<bool> Result;
		Result.Reserve(Count);
		Twister->Bool(Result, Count, 0);
		return Result;
	}
	
	UFUNCTION(BlueprintCallable)
	int32 Integer(int32 MinimumValue, int32 MaximumValue)
	{
		return Twister.Get()->IntegerRange(MinimumValue, MaximumValue);
	}
	
	FNMersenneTwister* GetTwister() const
	{
		return Twister.Get();
	}
	
	FNMersenneTwister& GetTwisterRef() const
	{
		return *Twister.Get();
	}
	
private:
	TSharedPtr<FNMersenneTwister> Twister;
};
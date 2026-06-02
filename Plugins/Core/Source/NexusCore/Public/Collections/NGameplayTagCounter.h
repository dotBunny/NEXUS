// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "GameplayTagContainer.h"
#include "NGameplayTagCounter.generated.h"

USTRUCT(BlueprintType)
struct FNGameplayTagCounter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, int32> GameplayTags;
	
	FNGameplayTagCounter()
	{
	}

	explicit FNGameplayTagCounter(const TMap<FGameplayTag, int32>& ExistingCounters)
	{
		GameplayTags.Append(ExistingCounters);
	}
	
	FNGameplayTagCounter(const FNGameplayTagCounter& Other)
	{
		*this = Other;
	}
	
	explicit FNGameplayTagCounter(const FGameplayTag& Tag)
	{
		Increment(Tag);
	}
	
	void Combine(FNGameplayTagCounter& Other)
	{
		for (const TPair<FGameplayTag, int32>& Pair : Other.GameplayTags)                                                             
		{
			GameplayTags.FindOrAdd(Pair.Key) += Pair.Value;
		}
	}
	
	void Increment(const FGameplayTag& Tag)
	{
		if (!GameplayTags.Contains(Tag))
		{
			GameplayTags.Add(Tag);
		}	
		GameplayTags[Tag]++;
	}
	
	void Decrement(const FGameplayTag& Tag)
	{
		if (!GameplayTags.Contains(Tag))
		{
			GameplayTags.Add(Tag);
		}	
		GameplayTags[Tag]--;
	}
	
	int32 GetCount(const FGameplayTag& Tag)
	{
		if (GameplayTags.Contains(Tag))
		{
			return GameplayTags[Tag];
		}
		return 0;
	}
};

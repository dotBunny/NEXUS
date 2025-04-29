// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NTestCollection.generated.h"

UCLASS(BlueprintType)
class UNTestCollection : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Developer")
	void SetDescription(FString NewDescription)
	{
		Description = NewDescription;
	}

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Developer")
	int AddTest(const FString& Identifier, const int TestExpectedPoints)
	{
		Identifiers.Add(Identifier);
		Current.Add(0);
		Expected.Add(TestExpectedPoints);

		return Current.Num() - 1;
	}

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Developer")
	void AddExpectedPoints(const int& Ticket, const int AdditionalExpectedPoints)
	{
		Expected[Ticket] += AdditionalExpectedPoints;
	}

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Developer")
	void AddPoints(const int& Ticket, const int Points)
	{
		Current[Ticket] += Points;
	}

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Developer")
	int GetExpectedPoints(const int& Ticket) const
	{
		return Expected[Ticket];
	}
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Developer")
	FString GetTestStatus(const int& Ticket) const
	{
		return FString::Printf(TEXT("%s %i/%i"), *Identifiers[Ticket], Current[Ticket], Expected[Ticket]);
	}

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Developer")
	FString GetStatus() const
	{
		int TotalPoints = 0;
		int TotalExpected = 0;

		for (const int Points : Current)
		{
			TotalPoints += Points;
		}
		for (const int Points : Expected)
		{
			TotalExpected += Points;
		}
		
		return FString::Printf(TEXT("%i/%i"), TotalPoints, TotalExpected);
	}

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Developer")
	bool IsPassed() const
	{
		int Count = Current.Num();
		for (int i = 0; i < Count; i++)
		{
			if (Current != Expected) return false;
		}
		return true;
	}

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Developer")
	bool IsExpected(const int& Ticket) const
	{
		return Current[Ticket] == Expected[Ticket];
	}
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Developer")
	void Reset()
	{
		Current.Empty();
		Expected.Empty();
	}
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Developer")
	int GetTestCount() const
	{
		return Current.Num();
	}

private:
	FString Description = "TestCollection";
	TArray<FString> Identifiers;
	TArray<int> Current;
	TArray<int> Expected;
};
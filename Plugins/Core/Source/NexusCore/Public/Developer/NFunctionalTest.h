// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "NTestCollection.h"
#include "NFunctionalTest.generated.h"

UCLASS(MinimalAPI, BlueprintType)
class ANFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()
	
	virtual void PrepareTest() override;
	virtual void OnTimeout() override;
	virtual void CleanUp() override;
	virtual void AddError(const FString& Message) override;

	UFUNCTION(BlueprintCallable, Category="NEXUS|Developer")
	void FinishTestCollection();
	

	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "NEXUS|Developer")
	void ReportExpectedFromCollection(const int& Ticket) const
	{
	
	}

	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "NEXUS|Developer")
	void AddPointsToCollection(const int& Ticket, const int Points) const
	{
		Collection->AddPoints(Ticket, Points);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "NEXUS|Developer")
	int AddTestToCollection(const FString& Identifier, const int ExpectedPoints)
	{
		return Collection->AddTest(Identifier, ExpectedPoints);
	}

	
private:
	UPROPERTY()
	TObjectPtr<UNTestCollection> Collection;

	bool bHasErrorOverride = false;
};

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NSamplesDisplayTest.generated.h"

UENUM(BlueprintType)
enum class ESampleDisplayTestResult : uint8
{
	Default = 0,
	Invalid = 1,
	Error = 2,
	Running = 3,
	Failed = 4,
	Succeeded  = 5
};

UCLASS(BlueprintType)
class UNSamplesDisplayTest : public UObject
{
	GENERATED_BODY()
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTestEventWithMessageSignature, const FString&, Message);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTestFinishEventSignature, ESampleDisplayTestResult, TestResult, const FString&, Message);

public:
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test")
	void AddWarning(const FString& Message) { OnTestWarning.Broadcast(Message); };
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test")
	void AddError(const FString& Message) { OnTestError.Broadcast(Message); };
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test")
	void AddInfo(const FString& Message) { OnTestInfo.Broadcast(Message); };
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test")
	void FinishTest(ESampleDisplayTestResult TestResult, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check True")
	void CheckTrue(const bool bResult, const FString FailMessage);
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check False")
	void CheckFalse(const bool bResult, const FString FailMessage);
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check True (w/ Count)")
	void CheckTrueWithCount(const bool bResult, const int& Count, const FString FailMessage);
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check False (w/ Count)")
	void CheckFalseWithCount(const bool bResult, const int& Count, const FString FailMessage);
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check True (w/ Location)")
	void CheckTrueWithLocation(const bool bResult, const FVector& Location, const FString FailMessage);
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check False (w/ Location)")
	void CheckFalseWithLocation(const bool bResult, const FVector& Location, const FString FailMessage);
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check True (w/ Object)")
	void CheckTrueWithObject(const bool bResult, const UObject* Object, const FString FailMessage);
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check False (w/ Object)")
	void CheckFalseWithObject(const bool bResult, const UObject* Object, const FString FailMessage);
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check True (w/ Actor)")
	void CheckTrueWithActor(const bool bResult, const AActor* Actor, const FString FailMessage);
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check False (w/ Actor)")
	void CheckFalseWithActor(const bool bResult, const AActor* Actor, const FString FailMessage);
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check Pass Count ++")
	void IncrementCheckPassCount() { CheckPassCount++; };
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test|Validation", DisplayName="Check Fail Count ++")
	void IncrementCheckFailCount() { CheckFailCount++; };
	
	UPROPERTY(BlueprintAssignable, Category = "NEXUS|Functional Test")
	FOnTestEventWithMessageSignature OnTestInfo;
	
	UPROPERTY(BlueprintAssignable, Category = "NEXUS|Functional Test")
	FOnTestEventWithMessageSignature OnTestError;
	
	UPROPERTY(BlueprintAssignable, Category = "NEXUS|Functional Test")
	FOnTestEventWithMessageSignature OnTestWarning;
	
	UPROPERTY(BlueprintAssignable, Category = "NEXUS|Functional Test")
	FOnTestFinishEventSignature OnTestFinish;
	
	void ClearBindings();
private:
	int CheckPassCount = 0;
	int CheckFailCount = 0;
};

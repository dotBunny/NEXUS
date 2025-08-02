// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

#if N_HAS_FUNCTIONAL_TESTING
#include "FunctionalTest.h"
#endif

#include "NFunctionalTestProxy.generated.h"

class USpotLightComponent;

#define N_TIMER_DRAW_THICKNESS 0.35f


UENUM(BlueprintType)
enum class ESampleTestResult : uint8
{
	Default,
	Invalid,
	Error,
	Running,
	Failed,
	Succeeded
};

UCLASS(MinimalAPI, BlueprintType)
class ANFunctionalTestProxy : public AActor
{
	DECLARE_DELEGATE(FOnFunctionalTestProxyEventSignature);
	
	GENERATED_BODY()

public:

	explicit ANFunctionalTestProxy(const FObjectInitializer& ObjectInitializer);
	void UpdateProxy(const FString& TestName);
	
	FOnFunctionalTestProxyEventSignature TestPrepared;
	UFUNCTION(BlueprintImplementableEvent, meta=( DisplayName="Prepare Test"))
	void ReceivePrepareTest();
	FOnFunctionalTestProxyEventSignature TestStarted;
	UFUNCTION(BlueprintImplementableEvent, meta=( DisplayName="Start Test"))
	void ReceiveStartTest();
	FOnFunctionalTestProxyEventSignature TestFinished;
	UFUNCTION(BlueprintImplementableEvent, meta=( DisplayName="Test Finished"))
	void ReceiveTestFinished();
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test")
	void AddWarning(const FString& Message);
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test")
	void AddError(const FString& Message);
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test")
	void AddInfo(const FString& Message);
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Functional Test")
	void FinishTest(ESampleTestResult TestResult, const FString& Message);

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
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Testing", DisplayName = "Use Functional Test Proxy")
	bool bUseFunctionalTestProxy = true;
	
	UPROPERTY()
	TObjectPtr<UChildActorComponent> TestActorComponent;
	
	int CheckPassCount = 0;
	int CheckFailCount = 0;
	
#if N_HAS_FUNCTIONAL_TESTING
	TObjectPtr<AFunctionalTest> FunctionalTestActor;
#else
	TObjectPtr<AActor> FunctionalTestActor;
#endif

private:
	UFUNCTION(Category = "NEXUS|Functional Test")
	void OnTestPrepare();
	UFUNCTION(Category = "NEXUS|Functional Test")
	void OnTestFinished();
	UFUNCTION(Category = "NEXUS|Functional Test")
	void OnTestStart();


	void CreateProxy();
	void RenameProxy(const FString& TestName) const;
	void DestroyProxy();
};
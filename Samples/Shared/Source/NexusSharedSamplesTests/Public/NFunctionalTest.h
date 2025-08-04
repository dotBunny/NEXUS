// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "NSamplesDisplayActor.h"
#include "NFunctionalTest.generated.h"


UCLASS(MinimalAPI, BlueprintType)
class ANFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NEXUS|References", DisplayName="Target Display")
	TSoftObjectPtr<ANSamplesDisplayActor> TargetDisplay;

	virtual void PrepareTest() override;
	virtual void StartTest() override;
	virtual void CleanUp() override;
	
protected:

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ANFunctionalTest, TargetDisplay))
		{
			UpdateFromDisplay();
		}
		Super::PostEditChangeProperty(PropertyChangedEvent);
	}
#endif

	
	void OnFinishTest(ESampleTestResult TestResult, const FString& Message);
	void UpdateFromDisplay();
	
private:
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	ANSamplesDisplayActor* Display;
};
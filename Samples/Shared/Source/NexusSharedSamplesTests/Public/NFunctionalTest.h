// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "NSamplesDisplayActor.h"
#include "NFunctionalTest.generated.h"


UCLASS(DisplayName = "NEXUS: Functional Test", MinimalAPI, BlueprintType)
class ANFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

public:
	explicit ANFunctionalTest(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NEXUS", DisplayName="Target Display")
	TSoftObjectPtr<ANSamplesDisplayActor> TargetDisplay;

	virtual void PrepareTest() override;
	virtual void StartTest() override;
	virtual void CleanUp() override;
	
protected:

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NEXUS")
	bool bHideBillboard = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NEXUS")
	bool bHideName= true;
#endif

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ANFunctionalTest, TargetDisplay))
		{
			UpdateFromDisplay();
		}
		Super::PostEditChangeProperty(PropertyChangedEvent);
	}
	void UpdateFromDisplay();
#endif
	
	UFUNCTION()
	void OnFinishSampleDisplayTest(ESampleDisplayTestResult TestResult, const FString& Message);

	
private:
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	ANSamplesDisplayActor* Display;
};
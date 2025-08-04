// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NFunctionalTest.h"

#include "Components/BillboardComponent.h"

ANFunctionalTest::ANFunctionalTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Display = nullptr;
	if (bHideBillboard)
	{
		GetSpriteComponent()->SetVisibility(false);
	}
	if (bHideName)
	{
		TestName->SetVisibility(false);
	}
}

void ANFunctionalTest::PrepareTest()
{
	Super::PrepareTest();

	// Work/link with Display
	Display = TargetDisplay.Get();
	
	if (Display != nullptr)
	{
		// Bindings
		Display->OnTestError.BindUObject(this, &AFunctionalTest::AddError);
		Display->OnTestInfo.BindUObject(this, &AFunctionalTest::AddInfo);
		Display->OnTestWarning.BindUObject(this, &AFunctionalTest::AddWarning);
		Display->OnTestFinish.BindUObject(this, &ANFunctionalTest::OnFinishTest);
		
		Display->PrepareTest();
	}
}

void ANFunctionalTest::StartTest()
{
	Super::StartTest();

	// Work with Display
	if (Display != nullptr)
	{
		Display->StartTest();
	}
}

void ANFunctionalTest::CleanUp()
{
	Super::CleanUp();

	// Work with Display
	if (Display != nullptr)
	{
		Display->CleanupTest();

		
		Display->OnTestError.Unbind();
		Display->OnTestInfo.Unbind();
		Display->OnTestWarning.Unbind();
		Display->OnTestFinish.Unbind();

		Display = nullptr;
	}
}

void ANFunctionalTest::OnFinishTest(ESampleTestResult TestResult, const FString& Message)
{
	FinishTest(static_cast<EFunctionalTestResult>(TestResult), Message);
}

void ANFunctionalTest::UpdateFromDisplay()
{
	const ANSamplesDisplayActor* Target = TargetDisplay.Get();
	if (Target != nullptr)
	{
		TestLabel = Target->GetTitle();
		this->SetActorLabel(TestLabel);
	}
}

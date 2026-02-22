// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NFunctionalTest.h"
#include "Components/BillboardComponent.h"

ANFunctionalTest::ANFunctionalTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Display = nullptr;
	
#if WITH_EDITORONLY_DATA
	if (bHideBillboard && GetSpriteComponent() != nullptr)
	{
		GetSpriteComponent()->SetVisibility(false);
	}
	if (bHideName && TestName != nullptr)
	{
		TestName->SetVisibility(false);
	}
#endif // WITH_EDITORONLY_DATA
}

void ANFunctionalTest::PrepareTest()
{
	Super::PrepareTest();

	// Work/link with Display
	Display = TargetDisplay.Get();
	
	if (Display != nullptr)
	{
		Display->PrepareTest();
		
		// Bind to test instance
		if (Display->TestInstance != nullptr)
		{
			Display->TestInstance->OnTestError.AddDynamic(this, &ANFunctionalTest::AddError);
			Display->TestInstance->OnTestWarning.AddDynamic(this, &ANFunctionalTest::AddWarning);
			Display->TestInstance->OnTestInfo.AddDynamic(this, &ANFunctionalTest::AddInfo);
			Display->TestInstance->OnTestFinish.AddDynamic(this, &ANFunctionalTest::OnFinishSampleDisplayTest);
		}
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
		Display = nullptr;
	}
}

void ANFunctionalTest::OnFinishSampleDisplayTest(ESampleDisplayTestResult TestResult, const FString& Message)
{
	switch (TestResult)
	{
		using enum ESampleDisplayTestResult;
	case Default:
		FinishTest(EFunctionalTestResult::Default, Message);
		break;
	case Invalid:
		FinishTest(EFunctionalTestResult::Invalid, Message);
		break;
	case Error:
		FinishTest(EFunctionalTestResult::Error, Message);
		break;
	case Running:
		FinishTest(EFunctionalTestResult::Running, Message);
		break;
	case Failed:
		FinishTest(EFunctionalTestResult::Failed, Message);
		break;
	case Succeeded:
		FinishTest(EFunctionalTestResult::Succeeded, Message);
		break;
	}
}

#if WITH_EDITOR
void ANFunctionalTest::UpdateFromDisplay()
{
	const ANSamplesDisplayActor* Target = TargetDisplay.Get();
	if (Target != nullptr)
	{
		TestLabel = Target->TitleSettings.TitleText.ToString();
		this->SetActorLabel(TestLabel);
	}
}
#endif // WITH_EDITOR

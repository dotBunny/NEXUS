// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NFunctionalTest.h"
#include "AutomationBlueprintFunctionLibrary.h"

void ANFunctionalTest::PrepareTest()
{
	if (Collection == nullptr)
	{
		Collection = NewObject<UNTestCollection>();
	}
	Collection->Reset();
	
	Super::PrepareTest();
}

void ANFunctionalTest::OnTimeout()
{
	if (Collection->GetTestCount() > 0)
	{
		AddError(Collection->GetStatus());
	}
	Super::OnTimeout();
}

void ANFunctionalTest::CleanUp()
{
	Super::CleanUp();

	// Cleanup Test Tracker
	if (Collection != nullptr)
	{
		Collection->ConditionalBeginDestroy();
		Collection = nullptr;
	}
}

void ANFunctionalTest::AddError(const FString& Message)
{
	bHasErrorOverride = true;
	Super::AddError(Message);
}

void ANFunctionalTest::FinishTestCollection()
{
	const int TicketLimit = Collection->GetTestCount() - 1;
	for (int i = 0; i < TicketLimit; i++)
	{
		if (Collection->IsExpected(i))
		{
			UAutomationBlueprintFunctionLibrary::AddTestInfo(*Collection->GetTestStatus(i));
		}
		else
		{
			UAutomationBlueprintFunctionLibrary::AddTestError(*Collection->GetTestStatus(i));
		}
	}
	
	if (!Collection->IsPassed() || bHasErrorOverride)
	{
		Super::FinishTest(EFunctionalTestResult::Failed, Collection->GetStatus());
	}
	else
	{
		Super::FinishTest(EFunctionalTestResult::Succeeded, Collection->GetStatus());
	}	
}

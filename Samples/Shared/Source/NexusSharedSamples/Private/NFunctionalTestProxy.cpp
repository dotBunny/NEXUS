#include "NFunctionalTestProxy.h"

#if N_HAS_FUNCTIONAL_TESTING
#include "FuncTestRenderingComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/TextRenderComponent.h"
#else
#include "NCoreMinimal.h"
#endif

ANFunctionalTestProxy::ANFunctionalTestProxy(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	TestActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("TestActor"));
}

void ANFunctionalTestProxy::InitProxy(const FString& TestName)
{
	// Disable and kill off references
	if (!bUseFunctionalTestProxy)
	{
		if (TestActorComponent != nullptr &&
			TestActorComponent->GetChildActor() != nullptr)
		{
			TestActorComponent->DestroyChildActor();
			FunctionalTestActor = nullptr;
		}
		return;
	}

	if (TestActorComponent != nullptr &&
		TestActorComponent->GetChildActor() == nullptr)
	{
#if N_HAS_FUNCTIONAL_TESTING
		TestActorComponent->SetChildActorClass(AFunctionalTest::StaticClass());
#else
		TestActorComponent->SetChildActorClass(AActor::StaticClass());
#endif
	}
	
#if N_HAS_FUNCTIONAL_TESTING
	if (FunctionalTestActor == nullptr)
	{
		FunctionalTestActor = Cast<AFunctionalTest>(TestActorComponent->GetChildActor());
		
		// Bindings
		FunctionalTestActor->OnTestPrepare.AddUniqueDynamic(this, &ANFunctionalTestProxy::OnTestPrepare);
		FunctionalTestActor->OnTestFinished.AddUniqueDynamic(this, &ANFunctionalTestProxy::OnTestFinished);
		FunctionalTestActor->OnTestStart.AddUniqueDynamic(this, &ANFunctionalTestProxy::OnTestStart);

#if WITH_EDITORONLY_DATA
		FunctionalTestActor->RenderComp->SetVisibility(false);
		FunctionalTestActor->TestName->SetVisibility(false);
		FunctionalTestActor->GetSpriteComponent()->SetVisibility(false);
#endif
	}
#endif

	// Handle updates
	if (FunctionalTestActor != nullptr)
	{
#if N_HAS_FUNCTIONAL_TESTING
		FunctionalTestActor->SetActorLabel(TestName);
#endif
	}
}

void ANFunctionalTestProxy::OnTestPrepare()
{
	TestPrepared.ExecuteIfBound();

	CheckPassCount = 0;
	CheckFailCount = 0;

	ReceivePrepareTest();
}

void ANFunctionalTestProxy::OnTestFinished()
{
	TestFinished.ExecuteIfBound();
	ReceiveTestFinished();
}

void ANFunctionalTestProxy::OnTestStart()
{
	TestStarted.ExecuteIfBound();
	ReceiveStartTest();
}


void ANFunctionalTestProxy::FinishTest(ESampleTestResult TestResult, const FString& Message)
{
	
	if (CheckPassCount > 0 || CheckFailCount > 0)
	{
		const FString UpdatedMessage = FString::Printf(TEXT("%s (PASS: %i | FAIL: %i)"), *Message.TrimStartAndEnd(), CheckPassCount, CheckFailCount);
		if (CheckFailCount > 0)
		{
#if N_HAS_FUNCTIONAL_TESTING			
			if (FunctionalTestActor)
			{
				FunctionalTestActor->FinishTest(static_cast<EFunctionalTestResult>(TestResult), UpdatedMessage);
			}
#else
			N_LOG(Error, TEXT("FAILED: %s"), *UpdatedMessage)
#endif
		}
		else
		{
#if N_HAS_FUNCTIONAL_TESTING			
			if (FunctionalTestActor)
			{
				FunctionalTestActor->FinishTest(static_cast<EFunctionalTestResult>(TestResult), UpdatedMessage);
			}
#else
			N_LOG(Error, TEXT("PASS: %s"), *UpdatedMessage)
#endif
		}
	}
	else
	{
#if N_HAS_FUNCTIONAL_TESTING			
		if (FunctionalTestActor)
		{
			FunctionalTestActor->FinishTest(static_cast<EFunctionalTestResult>(TestResult), Message);
		}
#else
		N_LOG(Warning, TEXT("NO CHECKS: %s"), *Message)
#endif
	}
}

void ANFunctionalTestProxy::AddWarning(const FString& Message)
{
#if N_HAS_FUNCTIONAL_TESTING				
	if (FunctionalTestActor)
	{
		FunctionalTestActor->AddWarning(Message);
	}
#endif
}

void ANFunctionalTestProxy::AddError(const FString& Message)
{
#if N_HAS_FUNCTIONAL_TESTING				
	if (FunctionalTestActor)
	{
		FunctionalTestActor->AddError(Message);
	}
#endif
}

void ANFunctionalTestProxy::AddInfo(const FString& Message)
{
#if N_HAS_FUNCTIONAL_TESTING				
	if (FunctionalTestActor)
	{
		FunctionalTestActor->AddInfo(Message);
	}
#endif
}



void ANFunctionalTestProxy::CheckTrue(const bool bResult, const FString FailMessage)
{
	if (bResult)
	{
		CheckPassCount++;
	}
	else
	{
		CheckFailCount++;
		AddError(FailMessage);
	}
}
void ANFunctionalTestProxy::CheckFalse(const bool bResult, const FString FailMessage)
{
	if (!bResult)
	{
		CheckPassCount++;
	}
	else
	{
		CheckFailCount++;
		AddError(FailMessage);
	}
}

void ANFunctionalTestProxy::CheckTrueWithCount(const bool bResult, const int& Count, const FString FailMessage)
{
	if (bResult)
	{
		CheckPassCount++;
	}
	else
	{
		CheckFailCount++;
		AddError(FString::Printf(TEXT("%s (%i)"), *FailMessage, Count));
	}
}

void ANFunctionalTestProxy::CheckFalseWithCount(const bool bResult, const int& Count, const FString FailMessage)
{
	if (!bResult)
	{
		CheckPassCount++;
	}
	else
	{
		CheckFailCount++;
		AddError(FString::Printf(TEXT("%s (%i)"), *FailMessage, Count));
	}
}

void ANFunctionalTestProxy::CheckTrueWithLocation(const bool bResult, const FVector& Location, const FString FailMessage)
{
	if (bResult)
	{
		CheckPassCount++;
	}
	else
	{
		CheckFailCount++;
		AddError(FString::Printf(TEXT("%s (%s)"), *FailMessage, *Location.ToCompactString()));
	}
}
void ANFunctionalTestProxy::CheckFalseWithLocation(const bool bResult, const FVector& Location, const FString FailMessage)
{
	if (!bResult)
	{
		CheckPassCount++;
	}
	else
	{
		CheckFailCount++;
		AddError(FString::Printf(TEXT("%s (%s)"), *FailMessage, *Location.ToCompactString()));
	}
}
void ANFunctionalTestProxy::CheckTrueWithObject(const bool bResult, const UObject* Object, const FString FailMessage)
{
	if (bResult)
	{
		CheckPassCount++;
	}
	else
	{
		CheckFailCount++;
		AddError(FString::Printf(TEXT("%s (%s)"), *FailMessage, *Object->GetName()));
	}
}
void ANFunctionalTestProxy::CheckFalseWithObject(const bool bResult, const UObject* Object, const FString FailMessage)
{
	if (!bResult)
	{
		CheckPassCount++;
	}
	else
	{
		CheckFailCount++;
		AddError(FString::Printf(TEXT("%s (%s)"), *FailMessage, *Object->GetName()));
	}
}
void ANFunctionalTestProxy::CheckTrueWithActor(const bool bResult, const AActor* Actor, const FString FailMessage)
{
	if (bResult)
	{
		CheckPassCount++;
	}
	else
	{
		CheckFailCount++;
		AddError(FString::Printf(TEXT("%s (%s)"), *FailMessage, *Actor->GetActorNameOrLabel()));
	}
}
void ANFunctionalTestProxy::CheckFalseWithActor(const bool bResult, const AActor* Actor, const FString FailMessage)
{
	if (!bResult)
	{
		CheckPassCount++;
	}
	else
	{
		CheckFailCount++;
		AddError(FString::Printf(TEXT("%s (%s)"), *FailMessage, *Actor->GetActorNameOrLabel()));
	}
}
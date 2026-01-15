#include "NSamplesDisplayTest.h"


void UNSamplesDisplayTest::FinishTest(ESampleDisplayTestResult TestResult, const FString& Message)
{
	if (CheckPassCount > 0 || CheckFailCount > 0)
	{
		const FString UpdatedMessage = FString::Printf(TEXT("%s (PASS: %i | FAIL: %i)"), *Message.TrimStartAndEnd(), CheckPassCount, CheckFailCount);
		OnTestFinish.Broadcast(TestResult, UpdatedMessage);
	}
	else
	{
		OnTestFinish.Broadcast(TestResult, Message);
	}
}

void UNSamplesDisplayTest::CheckTrue(const bool bResult, const FString FailMessage)
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
void UNSamplesDisplayTest::CheckFalse(const bool bResult, const FString FailMessage)
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

void UNSamplesDisplayTest::CheckTrueWithCount(const bool bResult, const int& Count, const FString FailMessage)
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

void UNSamplesDisplayTest::CheckFalseWithCount(const bool bResult, const int& Count, const FString FailMessage)
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

void UNSamplesDisplayTest::CheckTrueWithLocation(const bool bResult, const FVector& Location, const FString FailMessage)
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
void UNSamplesDisplayTest::CheckFalseWithLocation(const bool bResult, const FVector& Location, const FString FailMessage)
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
void UNSamplesDisplayTest::CheckTrueWithObject(const bool bResult, const UObject* Object, const FString FailMessage)
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
void UNSamplesDisplayTest::CheckFalseWithObject(const bool bResult, const UObject* Object, const FString FailMessage)
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
void UNSamplesDisplayTest::CheckTrueWithActor(const bool bResult, const AActor* Actor, const FString FailMessage)
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
void UNSamplesDisplayTest::CheckFalseWithActor(const bool bResult, const AActor* Actor, const FString FailMessage)
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

void UNSamplesDisplayTest::ClearBindings()
{
	OnTestInfo.Clear();
	OnTestWarning.Clear();
	OnTestFinish.Clear();
	OnTestError.Clear();
}

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NArrayUtils.h"
#include "Developer/NTestUObject.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(FNArrayUtilsTests_PinAll_EmptyInput, "NEXUS::UnitTests::NCore::FNArrayUtils::PinAll_EmptyInput", N_TEST_CONTEXT_ANYWHERE)
{
	const TArray<TWeakObjectPtr<UObject>> Objects;
	const TArray<TStrongObjectPtr<UObject>> Pinned = FNArrayUtils::PinAll(Objects);
	CHECK_MESSAGE(TEXT("An empty input should produce an empty output"), Pinned.Num() == 0);
}

N_TEST_HIGH(FNArrayUtilsTests_PinAll_ValidObjects, "NEXUS::UnitTests::NCore::FNArrayUtils::PinAll_ValidObjects", N_TEST_CONTEXT_ANYWHERE)
{
	UNTestUObject* First = NewObject<UNTestUObject>(GetTransientPackage());
	UNTestUObject* Second = NewObject<UNTestUObject>(GetTransientPackage());
	const TArray<TWeakObjectPtr<UObject>> Objects = {First, Second};

	const TArray<TStrongObjectPtr<UObject>> Pinned = FNArrayUtils::PinAll(Objects);
	CHECK_MESSAGE(TEXT("PinAll should produce one entry per input"), Pinned.Num() == 2);
	if (Pinned.Num() != 2) return;

	if (Pinned[0].Get() != First)
	{
		ADD_ERROR("The first pinned entry should reference the first source object.");
	}
	if (Pinned[1].Get() != Second)
	{
		ADD_ERROR("The second pinned entry should reference the second source object.");
	}
}

N_TEST_HIGH(FNArrayUtilsTests_PinAll_StaleWeakPointer, "NEXUS::UnitTests::NCore::FNArrayUtils::PinAll_StaleWeakPointer", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the documented contract: entries corresponding to stale weak pointers come back
	// null while valid entries keep their position in the output.
	UNTestUObject* Valid = NewObject<UNTestUObject>(GetTransientPackage());
	UNTestUObject* Stale = NewObject<UNTestUObject>(GetTransientPackage());
	const TArray<TWeakObjectPtr<UObject>> Objects = {Valid, Stale};

	Stale->MarkAsGarbage();

	const TArray<TStrongObjectPtr<UObject>> Pinned = FNArrayUtils::PinAll(Objects);
	CHECK_MESSAGE(TEXT("PinAll should produce one entry per input, stale or not"), Pinned.Num() == 2);
	if (Pinned.Num() != 2) return;

	if (Pinned[0].Get() != Valid)
	{
		ADD_ERROR("The valid entry should remain pinned at its original index.");
	}
	CHECK_FALSE_MESSAGE(TEXT("The stale entry should pin to null"), Pinned[1].IsValid());
}

#endif //WITH_TESTS

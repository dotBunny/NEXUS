// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_EDITORONLY_DATA

#include "CoreMinimal.h"
#include "Macros/NInterfaceMacros.h"
#include "NInterfaceMacrosTestTypes.generated.h"

/**
 * Test-only UINTERFACE used to exercise N_INTERFACE_INVOKE_METHOD variants. Exposes plain virtual
 * native methods (MethodName) alongside paired BlueprintImplementableEvent UFUNCTIONs (EventName)
 * so each macro arity has both branches to dispatch through.
 */
UINTERFACE()
class UNInterfaceMacrosTestSubject : public UInterface
{
	GENERATED_BODY()
};

/** Native interface half of UNInterfaceMacrosTestSubject; see that type for the dispatch matrix it exercises. */
class NEXUSCORETESTS_API INInterfaceMacrosTestSubject
{
	GENERATED_BODY()

public:
	// Native fast-path methods — plain virtuals so C++ implementers can override directly.
	virtual void OnZeroArg() {}
	virtual void OnOneArg(int32 Value) {}
	virtual void OnTwoArgs(int32 Value, const FString& Text) {}

	// Blueprint fallback hooks — UFUNCTIONs reached via Execute_<EventName> when the implementer is BP-only.
	UFUNCTION(BlueprintImplementableEvent)
	void OnZeroArgEvent();

	UFUNCTION(BlueprintImplementableEvent)
	void OnOneArgEvent(int32 Value);

	UFUNCTION(BlueprintImplementableEvent)
	void OnTwoArgsEvent(int32 Value, const FString& Text);
};

/** Native C++ implementer that records every invocation so tests can verify routing and argument forwarding. */
UCLASS()
class NEXUSCORETESTS_API UNInterfaceMacrosTestImplementer : public UObject, public INInterfaceMacrosTestSubject
{
	GENERATED_BODY()

public:
	int32 ZeroArgHits = 0;
	int32 OneArgHits = 0;
	int32 OneArgValue = 0;
	int32 TwoArgsHits = 0;
	int32 TwoArgsValue = 0;
	FString TwoArgsText;

	virtual void OnZeroArg() override { ++ZeroArgHits; }
	virtual void OnOneArg(int32 Value) override { ++OneArgHits; OneArgValue = Value; }
	virtual void OnTwoArgs(int32 Value, const FString& Text) override
	{
		++TwoArgsHits;
		TwoArgsValue = Value;
		TwoArgsText = Text;
	}
};

/** Bare UObject that does not implement the test interface; used to validate the macro's guard branch. */
UCLASS()
class NEXUSCORETESTS_API UNInterfaceMacrosTestNonImplementer : public UObject
{
	GENERATED_BODY()
};

/** Hosts the macro expansions so tests can call the generated Invoke<MethodName> statics. */
class FNInterfaceMacrosTestDispatcher
{
public:
	N_INTERFACE_INVOKE_METHOD(NInterfaceMacrosTestSubject, OnZeroArg, OnZeroArgEvent)
	N_INTERFACE_INVOKE_METHOD_OneParam(NInterfaceMacrosTestSubject, OnOneArg, OnOneArgEvent, int32, Value)
	N_INTERFACE_INVOKE_METHOD_TwoParams(NInterfaceMacrosTestSubject, OnTwoArgs, OnTwoArgsEvent, int32, Value, const FString&, Text)
};

#endif // WITH_EDITORONLY_DATA

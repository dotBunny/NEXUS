// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NWidgetState.h"

#include "NWidgetLibrary.generated.h"

/**
 * Blueprint-facing accessors for FNWidgetState. Bridges the C++ struct helpers into pure Kismet
 * nodes so save/restore flows can live entirely in blueprints.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Widget Library")
class UNWidgetLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** @return the boolean stored for Key in State, or false when missing. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|User Interface|Widget State", DisplayName="Get Boolean")
	static bool GetWidgetStateBoolean(const FNWidgetState& State, const FString& Key) { return State.GetBoolean(Key);}

	/** @return the float stored for Key in State, or 0 when missing. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|User Interface|Widget State", DisplayName="Get Float")
	static float GetWidgetStateFloat(const FNWidgetState& State, const FString& Key) { return State.GetFloat(Key);}

	/** @return the string stored for Key in State, or an empty string when missing. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|User Interface|Widget State", DisplayName="Get String")
	static FString GetWidgetStateString(const FNWidgetState& State, const FString& Key) { return State.GetString(Key);}

	/** Update or insert the boolean entry for Key on State. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|User Interface|Widget State", DisplayName="Set Boolean")
	static void SetWidgetStateBoolean(UPARAM(ref) FNWidgetState& State, const FString& Key, const bool Value) { State.SetBoolean(Key, Value); }

	/** Update or insert the float entry for Key on State. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|User Interface|Widget State", DisplayName="Set Float")
	static void SetWidgetStateFloat(UPARAM(ref) FNWidgetState& State, const FString& Key, const float Value) { State.SetFloat(Key, Value); }

	/** Update or insert the string entry for Key on State. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|User Interface|Widget State", DisplayName="Set String")
	static void SetWidgetStateString(UPARAM(ref) FNWidgetState& State, const FString& Key, const FString Value) { State.SetString(Key, Value); }
};
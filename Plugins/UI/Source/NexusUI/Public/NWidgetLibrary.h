// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NWidgetState.h"

#include "NWidgetLibrary.generated.h"

/**
 * @class UNWidgetLibrary
 * @brief A utility class providing functions NWidget types.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS: Widget Library")
class UNWidgetLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "NEXUS|User Interface|Widget State", DisplayName="Get Boolean")
	static bool GetWidgetStateBoolean(const FNWidgetState& State, const FString& Key) { return State.GetBoolean(Key);}
	UFUNCTION(BlueprintCallable, Category = "NEXUS|User Interface|Widget State", DisplayName="Get Float")
	static float GetWidgetStateFloat(const FNWidgetState& State, const FString& Key) { return State.GetFloat(Key);}
	UFUNCTION(BlueprintCallable, Category = "NEXUS|User Interface|Widget State", DisplayName="Get String")
	static FString GetWidgetStateString(const FNWidgetState& State, const FString& Key) { return State.GetString(Key);}
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|User Interface|Widget State", DisplayName="Set Boolean")
	static void SetWidgetStateBoolean(UPARAM(ref) FNWidgetState& State, const FString& Key, const bool Value) { State.SetBoolean(Key, Value); }
	UFUNCTION(BlueprintCallable, Category = "NEXUS|User Interface|Widget State", DisplayName="Set Float")
	static void SetWidgetStateFloat(UPARAM(ref) FNWidgetState& State, const FString& Key, const float Value) { State.SetFloat(Key, Value); }
	UFUNCTION(BlueprintCallable, Category = "NEXUS|User Interface|Widget State", DisplayName="Set String")
	static void SetWidgetStateString(UPARAM(ref) FNWidgetState& State, const FString& Key, const FString Value) { State.SetString(Key, Value); }
};
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "INActorPoolItem.h"
#include "NActorPoolLibrary.generated.h"

/**
 * @class UNActorPoolLibrary
 * A helper library exposing specific functionality for interacting with INActorPoolItems.
 */
UCLASS()
class UNActorPoolLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	/**
	 * An inconvenient way to bind to the OnActorOperationalStateChanged delegate on an INActorPoolItem.
	 * @note The function definition should be Function(const ENActorOperationalState OldState, const ENActorOperationalState NewState).
	 * @param Actor The target Actor which implements the INActorPoolItem interface.
	 * @param Object The UObject to bind to.
	 * @param FunctionName The function name to bind to.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Bind OnActorOperationalStateChanged", Category = "NEXUS|Actor Pool")
	static void BindOnActorOperationalStateChanged(const TScriptInterface<INActorPoolItem> Actor, UObject* Object, const FName FunctionName)
	{
		Actor->OnActorOperationalStateChanged.AddUFunction(Object, FunctionName);
	}

	/**
	 * Removes all bindings for the given Object to the OnActorOperationalStateChanged delegate.	 
	 * @param Actor The target Actor which implements the INActorPoolItem interface.
	 * @param Object The UObject to have its matched bindings removed.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Unbind OnActorOperationalStateChanged", Category = "NEXUS|Actor Pool")
	static void UnbindOnActorOperationalStateChanged(const TScriptInterface<INActorPoolItem> Actor, UObject* Object)
	{
		Actor->OnActorOperationalStateChanged.RemoveAll(Object);
	}
};
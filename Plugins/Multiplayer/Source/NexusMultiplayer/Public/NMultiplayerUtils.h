// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Macros/NWorldMacros.h"

class ALevelInstance;
/**
 * A collection of utility methods for multiplayer type things.
 */
class FNMultiplayerUtils
{
public:
	FORCEINLINE static void ServerTravel(const UObject *WorldContextObject, const FString& InURL, const bool bAbsolute = true, const bool bShouldSkipGameNotify = false)
	{
		if (UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject); IsValid(World))
		{
			World->ServerTravel(InURL, bAbsolute, bShouldSkipGameNotify);
		}
	}
	
	FORCEINLINE static bool IsMultiplayerTest()
	{
		return FParse::Param(FCommandLine::Get(), TEXT("NMultiplayerTest"));
	}

	FORCEINLINE static bool IsWorldAuthority(const UWorld* World)
	{
		return World->GetAuthGameMode() != nullptr;
	}

	FORCEINLINE static bool IsClient()
	{
		return GIsClient;
	}
};

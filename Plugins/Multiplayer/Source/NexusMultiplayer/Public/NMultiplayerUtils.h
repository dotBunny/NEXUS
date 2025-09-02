// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
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

	FORCEINLINE static bool HasWorldAuthority(const UWorld* World)
	{
		return World->GetAuthGameMode() != nullptr;
	}
	FORCEINLINE static bool HasWorldAuthority(const UWorld& World)
	{
		return World.GetAuthGameMode() != nullptr;
	}

	FORCEINLINE static bool HasGameStateAuthority(const UWorld* World)
	{
		const AGameStateBase* GameState = World->GetGameState();
		if (GameState == nullptr) return false;
		return GameState->GetLocalRole() == ROLE_Authority;
	}

	FORCEINLINE static int32 GetPlayerIdentifier(const APlayerController* PlayerController)
	{
		return PlayerController->GetPlayerState<APlayerState>()->GetPlayerId();;
	}

	FORCEINLINE static APawn* GetPawnFromPlayerIdentifier(const UWorld* World, const int32 PlayerIdentifier)
	{
		for (const auto PlayerState : World->GetGameState()->PlayerArray)
		{
			if (PlayerState->GetPlayerId() == PlayerIdentifier)
			{
				return PlayerState->GetPawn();
			}
		}
		return nullptr;
	}
};

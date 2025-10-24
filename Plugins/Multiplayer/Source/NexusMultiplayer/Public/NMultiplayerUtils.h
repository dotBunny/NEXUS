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
 * A handful of methods meant to support the building logic that works in multiplayer scenarios.
 * @see <a href="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/">FNMultiplayerUtils</a>
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

	/**
	 * Is the current session created from the MultiplayerTest editor command?
	 * @return true/false if it is.
	 */	
	FORCEINLINE static bool IsMultiplayerTest()
	{
		return FParse::Param(FCommandLine::Get(), TEXT("NMultiplayerTest"));
	}

	/**
	 * Does the current callstack have World authority?
	 * @remark Developer preference, use this to determine if logic is operating on the host.
	 * @param World The world to check.
	 * @return true/false if authority is found.
	 */		
	FORCEINLINE static bool HasWorldAuthority(const UWorld* World)
	{
		return World->GetAuthGameMode() != nullptr;
	}

	/**
	 * Does the current callstack have World authority?
	 * @remark Developer preference, use this to determine if logic is operating on the host.
	 * @param World The world to check.
	 * @return true/false if authority is found.
	 */		
	FORCEINLINE static bool HasWorldAuthority(const UWorld& World)
	{
		return World.GetAuthGameMode() != nullptr;
	}

	/**
	 * Does the current callstack have GameState authority?
	 * @remark One of many ways to check if the logic is being operated on the host/server.
	 * @param World The world to check.
	 * @return true/false if authority is found.
	 */
	FORCEINLINE static bool HasGameStateAuthority(const UWorld* World)
	{
		const AGameStateBase* GameState = World->GetGameState();
		if (GameState == nullptr) return false;
		return GameState->GetLocalRole() == ROLE_Authority;
	}

	/**
	* Does the current callstack have GameState authority?
	* @remark One of many ways to check if the logic is being operated on the host/server.
	* @param World The world to check.
	* @return true/false if authority is found.
	*/
	FORCEINLINE static bool HasGameStateAuthority(const UWorld& World)
	{
		const AGameStateBase* GameState = World.GetGameState();
		if (GameState == nullptr) return false;
		return GameState->GetLocalRole() == ROLE_Authority;
	}
	
	FORCEINLINE static int32 GetFirstPlayerIdentifier(const UWorld* World)
	{
		if (const AGameStateBase* GameState = World->GetGameState();
			GameState->PlayerArray.Num() > 0)
		{
			return GameState->PlayerArray[0]->GetPlayerId();
		}
		return 0;
	}
	
	FORCEINLINE static int32 GetPlayerIdentifier(const APlayerController* PlayerController)
	{
		return PlayerController->GetPlayerState<APlayerState>()->GetPlayerId();
	}

	FORCEINLINE static APawn* GetPawnFromPlayerIdentifier(const UWorld* World, const int32 PlayerIdentifier)
	{
		for (const AGameStateBase* GameState = World->GetGameState();
			const auto PlayerState : GameState->PlayerArray)
		{
			if (PlayerState->GetPlayerId() == PlayerIdentifier)
			{
				return PlayerState->GetPawn();
			}
		}
		return nullptr;
	}

	FORCEINLINE static AActor* GetPlayerControllerFromPlayerIdentifier(const UWorld* World, const int32 PlayerIdentifier)
	{
		for (const AGameStateBase* GameState = World->GetGameState();
			const auto PlayerState : GameState->PlayerArray)
		{
			if (PlayerState->GetPlayerId() == PlayerIdentifier)
			{
				return PlayerState->GetPlayerController();
			}
		}
		return nullptr;
	}
};

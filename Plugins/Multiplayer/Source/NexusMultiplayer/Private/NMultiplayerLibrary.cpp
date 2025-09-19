// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMultiplayerLibrary.h"

#include "NCoreMinimal.h"
#include "NMultiplayerUtils.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Macros/NWorldMacros.h"

float UNMultiplayerLibrary::Ping(const UObject* WorldContextObject)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (const ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController())
		{
			if (const APlayerController* PlayerController = LocalPlayer->GetPlayerController(World))
			{
				if (PlayerController->PlayerState != nullptr)
				{
					return PlayerController->PlayerState->ExactPing;
				}
			}
		}
	}
	return 0.0f;
}

bool UNMultiplayerLibrary::KickPlayer(UObject* WorldContextObject, APlayerState* PlayerState)
{
	if (PlayerState != nullptr && PlayerState->GetPlayerController() != nullptr && PlayerState->GetPlayerController()->HasAuthority())
	{
		N_LOG(Warning, TEXT("[UNMultiplayerLibrary::KickPlayer] Unable to kick host."))
		return false;
	}
	
	if (const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject))
	{
		// Not the server, sorry can't kick
		const AGameModeBase* GameMode = World->GetAuthGameMode();
		if (GameMode == nullptr || GameMode->GameSession == nullptr)
		{
			return false;
		}
		const FText KickReason = NSLOCTEXT("NetworkMessages", "HostKicked", "You were kicked.");
		return GameMode->GameSession->KickPlayer(PlayerState->GetPlayerController(), KickReason);
	}
	return false;
}

bool UNMultiplayerLibrary::HasRemotePlayers(UObject* WorldContextObject)
{
	if (const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject))
	{
		AGameModeBase* GameMode = World->GetAuthGameMode();
		if (GameMode == nullptr || GameMode->GameSession == nullptr)
		{
			return false;
		}
		for (const TObjectPtr<APlayerState> PlayerState : GameMode->GameState->PlayerArray)
		{
			if (PlayerState->GetPlayerController() != nullptr && PlayerState->GetPlayerController()->IsLocalController() == false)
			{
				return true;
			}
		}
	}
	return false;
}

bool UNMultiplayerLibrary::HasRemotePlayersExec(UObject* WorldContextObject)
{
	return HasRemotePlayers(WorldContextObject);
}

bool UNMultiplayerLibrary::HasLocalPlayersOnly(UObject* WorldContextObject)
{
	return !HasRemotePlayers(WorldContextObject);
}

bool UNMultiplayerLibrary::HasLocalPlayersOnlyExec(UObject* WorldContextObject)
{
	return HasLocalPlayersOnly(WorldContextObject);
}

bool UNMultiplayerLibrary::HasGameStateAuthority(UObject* WorldContextObject)
{
	if (const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject))
	{
		return FNMultiplayerUtils::HasGameStateAuthority(World);
	}
	return true;
}

bool UNMultiplayerLibrary::HasGameStateAuthorityExec(UObject* WorldContextObject)
{
	return HasGameStateAuthority(WorldContextObject);
}

bool UNMultiplayerLibrary::HasWorldAuthority(UObject* WorldContextObject)
{
	if (const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject))
	{
		return FNMultiplayerUtils::HasWorldAuthority(World);
	}
	return true;
}

bool UNMultiplayerLibrary::HasWorldAuthorityExec(UObject* WorldContextObject)
{
	return HasWorldAuthority(WorldContextObject);
}

bool UNMultiplayerLibrary::IsServer(UObject* WorldContextObject)
{
	const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);
	return World ? (World->GetNetMode() != NM_Client) : false;
}

bool UNMultiplayerLibrary::IsServerExec(UObject* WorldContextObject)
{
	return IsServer(WorldContextObject);
}

bool UNMultiplayerLibrary::IsMultiplayerTest()
{
	return FNMultiplayerUtils::IsMultiplayerTest();
}

bool UNMultiplayerLibrary::IsMultiplayerTestExec()
{
	return FNMultiplayerUtils::IsMultiplayerTest();
}
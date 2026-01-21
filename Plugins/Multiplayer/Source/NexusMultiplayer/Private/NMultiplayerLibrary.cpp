// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMultiplayerLibrary.h"

#include "NMultiplayerMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Macros/NWorldMacros.h"

float UNMultiplayerLibrary::Ping(const UObject* WorldContextObject)
{
	return FNMultiplayerUtils::GetPing(GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull));
}

bool UNMultiplayerLibrary::KickPlayer(UObject* WorldContextObject, APlayerState* PlayerState)
{
	if (PlayerState != nullptr && PlayerState->GetPlayerController() != nullptr && PlayerState->GetPlayerController()->HasAuthority())
	{
		UE_LOG(LogNexusMultiplayer, Error, TEXT("You are unable to kick the host."))
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
		const FText KickReason = NSLOCTEXT("NexusMultiplayer", "HostKicked", "You were kicked.");
		return GameMode->GameSession->KickPlayer(PlayerState->GetPlayerController(), KickReason);
	}
	return false;
}

bool UNMultiplayerLibrary::HasRemotePlayers(UObject* WorldContextObject)
{
	if (const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject))
	{
		const AGameModeBase* GameMode = World->GetAuthGameMode();
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
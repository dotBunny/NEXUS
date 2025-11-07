// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NMultiplayerUtils.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NMultiplayerLibrary.generated.h"

class APlayerState;

/**
 * A handful of methods meant to support the building logic that works in multiplayer scenarios.
 * @see <a href="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/">UNMultiplayerLibrary</a>
 */
UCLASS()
class NEXUSMULTIPLAYER_API UNMultiplayerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/**
	 * Get the current ping to the host/server.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return The numerical ping (ms) to the session host.
	 */
	UFUNCTION(BlueprintPure, DisplayName = "Get Ping", Category = "NEXUS|Multiplayer", meta = (WorldContext = "WorldContextObject",
		DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#ping"))
	static float Ping(const UObject* WorldContextObject);

	/**
	 * Kicks a player from a session.
	 * @remark This should be only called on the server or world owner.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @param PlayerState The target player to kick.
	 * @return Was the player able to be kicked? true/false.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Kick Player", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#kick-player"))
	static bool KickPlayer(UObject* WorldContextObject, APlayerState* PlayerState);

	/**
	 * Does the current world have remotely connected clients to it?
	 * @remark Clients won't have remote connections, only the server will.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if remote clients are found.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Has Remote Players", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#has-remote-players"))
	static bool HasRemotePlayers(UObject* WorldContextObject);

	/**
	 * Does the current world have remotely connected clients to it?
	 * @remark Clients won't have remote connections, only the server will.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if remote clients are found.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "? Has Remote Players", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#has-remote-players"))
	static bool HasRemotePlayersExec(UObject* WorldContextObject)
	{
		return HasRemotePlayers(WorldContextObject);
	};

	/**
	 * Does the current world have locally connected clients only?
	 * @remark Couch Co-op.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if only local clients are found.
	 */	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Has Local Players", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#has-local-players-only"))
	static bool HasLocalPlayersOnly(UObject* WorldContextObject)
	{
		return !HasRemotePlayers(WorldContextObject);
	};

	/**
	 * Does the current world have locally connected clients only?
	 * @remark Couch Co-op.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if only local clients are found.
	 */		
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "? Has Local Players", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#has-local-players-only"))
	static bool HasLocalPlayersOnlyExec(UObject* WorldContextObject)
	{
		return HasLocalPlayersOnly(WorldContextObject);
	};

	/**
	 * Does the current callstack have GameState authority?
	 * @remark One of many ways to check if the logic is being operated on the host/server.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if authority is found.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Has GameState Authority", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#has-gamestate-authority"))
	static bool HasGameStateAuthority(UObject* WorldContextObject)
	{
		if (const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject))
		{
			return FNMultiplayerUtils::HasGameStateAuthority(World);
		}
		return true;
	}

	/**
	 * Does the current callstack have GameState authority?
	 * @remark One of many ways to check if the logic is being operated on the host/server.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if authority is found.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "? Has GameState Authority", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#has-gamestate-authority"))
	static bool HasGameStateAuthorityExec(UObject* WorldContextObject)
	{
		return HasGameStateAuthority(WorldContextObject);
	}

	/**
	 * Does the current callstack have World authority?
	 * @remark Developer preference, use this to determine if logic is operating on the host.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if authority is found.
	 */	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Has World Authority", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#has-world-authority"))
	static bool HasWorldAuthority(UObject* WorldContextObject)
	{
		if (const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject))
		{
			return FNMultiplayerUtils::HasWorldAuthority(World);
		}
		return true;
	};

	/**
	 * Does the current callstack have World authority?
	 * @remark Developer preference, use this to determine if logic is operating on the host.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if authority is found.
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "? Has World Authority", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#has-world-authority"))
	static bool HasWorldAuthorityExec(UObject* WorldContextObject)
	{
		return HasWorldAuthority(WorldContextObject);
	};

	/**
	 * Get a player's unique identifier from the APlayerController.
	 * @param PlayerController The target APlayerController to use when querying for the player identification number.
	 * @return The player's identifier.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Get PlayerIdentifier", Category = "NEXUS|Multiplayer",
			meta = (DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#get-playeridentifier"))
	static int32 GetPlayerIdentifier(const APlayerController* PlayerController)
	{
		return PlayerController->GetPlayerState<APlayerState>()->GetPlayerId();
	}

	/**
	 * Get the first player's unique identifier.
	 * @param WorldContextObject An object to get the UWorld from.
	 * @return The player's identifier.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Get First PlayerIdentifier", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#get-first-playeridentifier"))
	static int32 GetFirstPlayerIdentifier(UObject* WorldContextObject)
	{
		if (const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject))
		{
			return FNMultiplayerUtils::GetFirstPlayerIdentifier(World);
		}
		return 0;
	}

	/**
	 * Get the APawn for the given player's unique identifier.
	 * @param WorldContextObject An object to get the UWorld from.
	 * @param PlayerIdentifier The target identifier to query for.
	 * @return If found, APawn, or nullptr.
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "Get Pawn From PlayerIdentifier", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#get-pawn-from-playeridentifier"))
	static APawn* GetPawnFromPlayerIdentifier(UObject* WorldContextObject, const int32 PlayerIdentifier)
	{
		if (const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject))
		{
			return FNMultiplayerUtils::GetPawnFromPlayerIdentifier(World, PlayerIdentifier);
		}
		return nullptr;
	}

	/**
	 * Get the AActor for the given player's unique identifier.
	 * @param WorldContextObject An object to get the UWorld from.
	 * @param PlayerIdentifier The target identifier to query for.
	 * @return If found, AActor, or nullptr.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Get PlayerController From PlayerIdentifier", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#get-playercontroller-from-playeridentifier"))
	static AActor* GetPlayerControllerFromPlayerIdentifier(UObject* WorldContextObject, const int32 PlayerIdentifier)
	{
		if (const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject))
		{
			return FNMultiplayerUtils::GetPlayerControllerFromPlayerIdentifier(World, PlayerIdentifier);
		}
		return nullptr;
	}
	
	/**
	 * Get the APlayerState for the given player's unique identifier.
	 * @param WorldContextObject An object to get the UWorld from.
	 * @param PlayerIdentifier The target identifier to query for.
	 * @return If found, APlayerState, or nullptr.
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "Get PlayerState From PlayerIdentifier", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#get-playerstate-from-playeridentifier"))
	static APlayerState* GetPlayerStateFromPlayerIdentifier(UObject* WorldContextObject, const int32 PlayerIdentifier)
	{
		if (const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject))
		{
			return FNMultiplayerUtils::GetPlayerStateFromPlayerIdentifier(World, PlayerIdentifier);
		}
		return nullptr;
	}

	/**
	 * An explicit check that the network mode of the world is not NM_Client, thus either a listen server (w/ client) or a dedicated server.	 
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if the world is not operating in NM_Client mode.
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "Is Server", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#is-server"))
	static bool IsServer(UObject* WorldContextObject)
	{
		const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);
		return World ? (World->GetNetMode() != NM_Client) : false;
	};

	/**
	 * An explicit check that the network mode of the world is not NM_Client, thus either a listen server (w/ client) or a dedicated server.	 
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if the world is not operating in NM_Client mode.
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "? Is Server", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#is-server"))
	static bool IsServerExec(UObject* WorldContextObject)
	{
		return IsServer(WorldContextObject);
	};

	/**
	 * Is the current session created from the MultiplayerTest editor command?
	 * @return true/false if it is.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Is Multiplayer Test", Category = "NEXUS|Multiplayer",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#is-multiplayer-test"))
	static bool IsMultiplayerTest()
	{
		return FNMultiplayerUtils::IsMultiplayerTest();
	};

	/**
	* Is the current session created from the MultiplayerTest editor command?
	* @return true/false if it is.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "? Is Multiplayer Test", Category = "NEXUS|Multiplayer",
		meta = (ExpandBoolAsExecs="ReturnValue", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#is-multiplayer-test"))
	static bool IsMultiplayerTestExec()
	{
		return FNMultiplayerUtils::IsMultiplayerTest();
	}
};

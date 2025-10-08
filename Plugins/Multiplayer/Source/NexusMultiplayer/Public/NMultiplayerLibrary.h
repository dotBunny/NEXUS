// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
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
	static bool HasRemotePlayersExec(UObject* WorldContextObject);

	/**
	 * Does the current world have locally connected clients only?
	 * @remark Couch Co-op.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if only local clients are found.
	 */	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Has Local Players", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#has-local-players-only"))
	static bool HasLocalPlayersOnly(UObject* WorldContextObject);

	/**
	 * Does the current world have locally connected clients only?
	 * @remark Couch Co-op.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if only local clients are found.
	 */		
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "? Has Local Players", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#has-local-players-only"))
	static bool HasLocalPlayersOnlyExec(UObject* WorldContextObject);

	/**
	 * Does the current callstack have GameState authority?
	 * @remark One of many ways to check if the logic is being operated on the host/server.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if authority is found.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Has GameState Authority", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#has-gamestate-authority"))
	static bool HasGameStateAuthority(UObject* WorldContextObject);

	/**
	 * Does the current callstack have GameState authority?
	 * @remark One of many ways to check if the logic is being operated on the host/server.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if authority is found.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "? Has GameState Authority", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#has-gamestate-authority"))
	static bool HasGameStateAuthorityExec(UObject* WorldContextObject);

	/**
	 * Does the current callstack have World authority?
	 * @remark Developer preference, use this to determine if logic is operating on the host.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if authority is found.
	 */	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Has World Authority", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#has-world-authority"))
	static bool HasWorldAuthority(UObject* WorldContextObject);

	/**
	 * Does the current callstack have World authority?
	 * @remark Developer preference, use this to determine if logic is operating on the host.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if authority is found.
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "? Has World Authority", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#has-world-authority"))
	static bool HasWorldAuthorityExec(UObject* WorldContextObject);

	/**
	 * An explicit check that the network mode of the world is not NM_Client, thus either a listen server (w/ client) or a dedicated server.	 
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if the world is not operating in NM_Client mode.
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "Is Server", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#is-server"))
	static bool IsServer(UObject* WorldContextObject);

	/**
	 * An explicit check that the network mode of the world is not NM_Client, thus either a listen server (w/ client) or a dedicated server.	 
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @return true/false if the world is not operating in NM_Client mode.
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "? Is Server", Category = "NEXUS|Multiplayer",
		meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#is-server"))
	static bool IsServerExec(UObject* WorldContextObject);

	/**
	 * Is the current session created from the MultiplayerTest editor command?
	 * @return true/false if it is.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Is Multiplayer Test", Category = "NEXUS|Multiplayer",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#is-multiplayer-test"))
	static bool IsMultiplayerTest();

	/**
	* Is the current session created from the MultiplayerTest editor command?
	* @return true/false if it is.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "? Is Multiplayer Test", Category = "NEXUS|Multiplayer",
		meta = (ExpandBoolAsExecs="ReturnValue", DocsURL="https://nexus-framework.com/docs/plugins/multiplayer/types/multiplayer-library/#is-multiplayer-test"))
	static bool IsMultiplayerTestExec();
};

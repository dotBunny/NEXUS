// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NMultiplayerLibrary.generated.h"

class APlayerState;

UCLASS()
class NEXUSMULTIPLAYER_API UNMultiplayerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, DisplayName = "Get Ping", Category = "NEXUS|Multiplayer", meta = (WorldContext = "WorldContextObject"))
	static float Ping(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Kick Player", Category = "NEXUS|Multiplayer", meta = (WorldContext = "WorldContextObject"))
	static bool KickPlayer(UObject* WorldContextObject, APlayerState* PlayerState);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Has Remote Players", Category = "NEXUS|Multiplayer", meta = (WorldContext = "WorldContextObject"))
	static bool HasRemotePlayers(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "? Has Remote Players", Category = "NEXUS|Multiplayer", meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue"))
	static bool HasRemotePlayersExec(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Has Local Players", Category = "NEXUS|Multiplayer", meta = (WorldContext = "WorldContextObject"))
	static bool HasLocalPlayersOnly(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "? Has Local Players", Category = "NEXUS|Multiplayer", meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue"))
	static bool HasLocalPlayersOnlyExec(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Has GameState Authority", Category = "NEXUS|Multiplayer", meta = (WorldContext = "WorldContextObject"))
	static bool HasGameStateAuthority(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, DisplayName = "? Has GameState Authority", Category = "NEXUS|Multiplayer", meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue"))
	static bool HasGameStateAuthorityExec(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Has World Authority", Category = "NEXUS|Multiplayer", meta = (WorldContext = "WorldContextObject"))
	static bool HasWorldAuthority(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, DisplayName = "? Has World Authority", Category = "NEXUS|Multiplayer", meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue"))
	static bool HasWorldAuthorityExec(UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, DisplayName = "? Is Host", Category = "NEXUS|Multiplayer", meta = (WorldContext = "WorldContextObject", ExpandBoolAsExecs="ReturnValue"))
	static bool IsServerExec(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Is Multiplayer Test", Category = "NEXUS|Multiplayer")
	static bool IsMultiplayerTest();

	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "? Is Multiplayer Test", Category = "NEXUS|Multiplayer", meta = (ExpandBoolAsExecs="ReturnValue"))
	static bool IsMultiplayerTestExec();

};

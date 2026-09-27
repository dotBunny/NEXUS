// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "NNullGameMode.generated.h"

/**
 * A game mode that does nothing but hand the player an ANMenuPlayerController.
 *
 * Suited to a bootstrap, splash or front-end map that has no gameplay of its own: no pawn is needed, only UI input
 * reaches the player, and seamless travel is off so leaving the map is a clean hard travel.
 * @see <a href="https://nexus-framework.com/docs/core/types/framework/null-game-mode/">ANNullGameMode</a>
 */
UCLASS(DisplayName="NEXUS | Null GameMode")
class NEXUSCORE_API ANNullGameMode : public AGameMode
{
	GENERATED_BODY()

	ANNullGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

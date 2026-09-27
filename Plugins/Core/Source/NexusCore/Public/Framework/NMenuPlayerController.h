// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NMenuPlayerController.generated.h"

/**
 * A player controller for menu-only maps: UI input only, no gameplay input, no tick.
 *
 * On BeginPlay it switches to FInputModeUIOnly and disables its own input, and it binds nothing in
 * SetupInputComponent, so only widgets receive input. On EndPlay it clears the game viewport's ignore-input flag that
 * FInputModeUIOnly sets, because the viewport client outlives this world and would otherwise swallow all input in
 * whatever map is travelled to next.
 * @see <a href="https://nexus-framework.com/docs/core/types/framework/menu-player-controller/">ANMenuPlayerController</a>
 */
UCLASS(DisplayName="NEXUS | Menu PlayerController")
class NEXUSCORE_API ANMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANMenuPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
};
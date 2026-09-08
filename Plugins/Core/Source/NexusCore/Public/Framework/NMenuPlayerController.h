// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NMenuPlayerController.generated.h"

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
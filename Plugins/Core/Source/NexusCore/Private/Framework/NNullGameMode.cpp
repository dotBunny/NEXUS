#include "Framework/NNullGameMode.h"

#include "Framework/NMenuPlayerController.h"

ANNullGameMode::ANNullGameMode(const FObjectInitializer& ObjectInitializer)
{
	PlayerControllerClass = ANMenuPlayerController::StaticClass();
	bUseSeamlessTravel = false;
}

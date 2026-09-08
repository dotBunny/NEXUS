#include "Framework/NMenuPlayerController.h"

#include "Engine/GameViewportClient.h"
#include "Engine/World.h"

ANMenuPlayerController::ANMenuPlayerController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ANMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetInputMode(FInputModeUIOnly());
	DisableInput(this);
}

void ANMenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// FInputModeUIOnly sets UGameViewportClient::bIgnoreInput, and the viewport client is owned by the game instance
	// rather than this world -- leaving the flag set swallows all input in whatever map is travelled to next. Clear
	// just that flag instead of imposing a whole input mode; the next controller decides what it actually wants.
	if (const UWorld* World = GetWorld())
	{
		if (UGameViewportClient* ViewportClient = World->GetGameViewport())
		{
			ViewportClient->SetIgnoreInput(false);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ANMenuPlayerController::SetupInputComponent()
{

}
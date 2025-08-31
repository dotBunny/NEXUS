// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSamplesGameMode.h"

#include "NSamplesHUD.h"
#include "NSamplesPawn.h"

ANSamplesGameMode::ANSamplesGameMode(const FObjectInitializer& ObjectInitializer)
{
	DefaultPawnClass = ANSamplesPawn::StaticClass();
	HUDClass = ANSamplesHUD::StaticClass();
}
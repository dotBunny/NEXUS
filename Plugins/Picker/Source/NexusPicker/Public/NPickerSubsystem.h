// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NSubsystemMacros.h"
#include "NPickerSubsystem.generated.h"

UCLASS()
class NEXUSPICKER_API UNPickerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	N_WORLD_SUBSYSTEM_GAME_ONLY(UNPickerSubsystem, true)
};
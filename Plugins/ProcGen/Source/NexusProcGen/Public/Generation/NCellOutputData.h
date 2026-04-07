// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Cell/NCell.h"

struct NEXUSPROCGEN_API FNCellOutputData
{
	FVector WorldPosition;
	FRotator WorldRotation;
	
	// MAIN-THREAD USE ONLY // ROOT OBJECT NEEDS TO BE UNLOADED LATER
	TObjectPtr<UNCell> Template;
};

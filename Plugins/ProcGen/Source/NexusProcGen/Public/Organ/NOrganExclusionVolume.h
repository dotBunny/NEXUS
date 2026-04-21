// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameFramework/Volume.h"
#include "NOrganExclusionVolume.generated.h"

/**
 * Volume actor that carves negative space out of overlapping organs.
 *
 * ProcGen skips cell placement anywhere an exclusion volume overlaps an organ — useful for
 * reserving space for hand-authored set-pieces, sub-levels, or hazards inside an otherwise
 * procedurally-populated area.
 */
UCLASS(BlueprintType, ClassGroup = "NEXUS", DisplayName = "NEXUS | Organ Exclusion Volume", HideCategories=(Tags, Activation, Cooking,
	AssetUserData, Navigation, Actor, Input))
class NEXUSPROCGEN_API ANOrganExclusionVolume : public AVolume
{
	GENERATED_BODY()
};
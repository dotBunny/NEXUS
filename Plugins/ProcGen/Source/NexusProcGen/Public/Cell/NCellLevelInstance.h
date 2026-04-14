// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellJunctionDetails.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "NCellLevelInstance.generated.h"

UCLASS(NotPlaceable, HideDropdown, Hidden, Transient, ClassGroup = "NEXUS", DisplayName = "NEXUS | Cell LevelInstance")
class NEXUSPROCGEN_API ANCellLevelInstance final : public ALevelInstance
{
	GENERATED_BODY()

public:
	TMap<int32, FNCellJunctionDetails>* JunctionData;
	
	virtual void OnLevelInstanceLoaded() override;

	/**
	 * Expose ability to set the CookedWorldAsset in build so that LevelInstance is able to load the level.
	 * @note At author time we use the SetWorldAsset method, but that no-ops in build.
	 * @param InWorldAsset World asset reference.
	 */
	void SetCookedWorldAsset(TSoftObjectPtr<UWorld> InWorldAsset)
	{
		CookedWorldAsset = InWorldAsset;
	}
};

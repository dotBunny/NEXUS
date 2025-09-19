﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "NCellLevelInstance.generated.h"

//DECLARE_MULTICAST_DELEGATE_OneParam(FOnNCellLevelLoaded, ANCellActor* NCellActor);

UCLASS(NotPlaceable, HideDropdown, Hidden, Transient, DisplayName = "NCell LevelInstance")
class NEXUSPROCGEN_API ANCellLevelInstance final : public ALevelInstance
{
	GENERATED_BODY()
public:
	// FOnNCellLevelLoaded OnNCellLevelLoaded;
protected:
	virtual void OnLevelInstanceLoaded() override;
};

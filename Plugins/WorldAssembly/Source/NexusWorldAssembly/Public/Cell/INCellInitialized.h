// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellLevelInstance.h"
#include "INCellInitialized.generated.h"

UINTERFACE()
class NEXUSWORLDASSEMBLY_API UNCellInitialized : public UInterface
{
	GENERATED_BODY()
};


class NEXUSWORLDASSEMBLY_API INCellInitialized
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category="NEXUS|World Assembly")
	void OnInitializedFromProxy(ANCellLevelInstance* CellLevelInstance);
};
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellJunctionComponent.h"
#include "INCellJunctionConnector.generated.h"

UINTERFACE()
class NEXUSWORLDASSEMBLY_API UNCellJunctionConnector : public UInterface
{
	GENERATED_BODY()
};

class NEXUSWORLDASSEMBLY_API INCellJunctionConnector
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category="NEXUS|World Assembly")
	void OnConnectJunctions(
		ANCellLevelInstance* StartCellLevelInstance, UNCellJunctionComponent* StartJunctionComponent,  int32 StartJunctionIndex,
		ANCellLevelInstance* EndCellLevelInstance, UNCellJunctionComponent* EndJunctionComponent,  int32 EndJunctionIndex);
};
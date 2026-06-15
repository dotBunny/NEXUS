// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellLinkDetails.h"
#include "INCellJunctionBeginPlay.generated.h"

UINTERFACE()
class NEXUSWORLDASSEMBLY_API UNCellJunctionBeginPlay : public UInterface
{
	GENERATED_BODY()
};

class NEXUSWORLDASSEMBLY_API INCellJunctionBeginPlay
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category="NEXUS|World Assembly")
	void OnJunctionBeginPlay(FNCellLinkDetails CellLinkDetails);
};
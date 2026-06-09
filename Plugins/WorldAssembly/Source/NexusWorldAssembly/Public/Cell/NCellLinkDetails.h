// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCellLinkDetails.generated.h"

USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNCellLinkDetails
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 JunctionInstanceIdentifier = -1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bConnected = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ConnectedNodeIndex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ConnectedJunctionInstanceIdentifier;	
};
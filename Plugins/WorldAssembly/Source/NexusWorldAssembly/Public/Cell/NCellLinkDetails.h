// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCellLinkDetails.generated.h"

USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNCellLinkDetails
{
	GENERATED_BODY()
	
	/** The unique cell identifier of the junction this link represents. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 JunctionInstanceIdentifier = -1;
	
	/** Was this junction connected/filled? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bConnected = false;

	/** The Node Identifier of the cell the junction connects to via its junction. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ConnectedNodeIdentifier = -1;
	
	/** The unique cell identifier that the junction this link represents connects to. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ConnectedJunctionInstanceIdentifier = -1;	

	/** Does this junction connect two cells that both lie on the shortest-path hot path (spokes from start). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHotPathShortest = false;

	/** Does this junction connect two cells that both lie on the sequential hot path (visiting chain). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHotPathSequential = false;
};
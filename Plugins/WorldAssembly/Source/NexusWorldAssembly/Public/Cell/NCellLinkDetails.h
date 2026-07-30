// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCellLinkDetails.generated.h"

/**
 * The resolved connection state of a single junction once the cell graph has been linked.
 *
 * Identifies the junction (and the node owning it), whether it ended up connected, and if so which
 * node and junction it reaches. The two hot-path flags describe the link rather than either cell:
 * they are set only when the connection itself forms part of the shortest or sequential hot path.
 * Handed to implementors of INCellJunctionBeginPlay so gameplay can react to how a junction was wired.
 */
USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNCellLinkDetails
{
	GENERATED_BODY()

	/** The unique cell identifier of the junction this link represents. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 JunctionInstanceIdentifier = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 NodeIdentifier = -1;

	/** Was this junction connected/filled to another cell/bone? */
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

	FString ToString() const
	{
		return FString::Printf(
			TEXT("[%d:%d] Connected: %s > [%d:%d] | HotShort: %s | HotSeq: %s"),
			NodeIdentifier,
			JunctionInstanceIdentifier,
			bConnected ? TEXT("True") : TEXT("False"),
			ConnectedNodeIdentifier,
			ConnectedJunctionInstanceIdentifier,
			bHotPathShortest ? TEXT("True") : TEXT("False"),
			bHotPathSequential ? TEXT("True") : TEXT("False")
		);
	}
};
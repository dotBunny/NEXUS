// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWorldAssemblyMinimal.h"
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

	/**
	 * Was this junction paired by the connector pass rather than by two cells mating directly?
	 * @note bConnected is also true for these, so the junction is not filled — but nothing occupies the opening
	 *       until the connector actor spawns, which happens once both ends of the pairing have streamed in.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bConnector = false;

	/**
	 * Identifier of the connector pairing this junction belongs to, or INDEX_NONE when it is not connector-paired.
	 * @note Both ends of a pairing carry the same value, and this — not ConnectedNodeIdentifier — is what rejoins
	 *       them at runtime: node identifiers are only unique within a single assembly graph, and a connector
	 *       pairing can span graphs.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ConnectorIdentifier = INDEX_NONE;

	/**
	 * The proximity scores of the cell on the *far* side of this junction, as they appear on that cell's
	 * FNCellAssemblyData. Left at UnreachableScore when the junction is unconnected or reaches a bone.
	 *
	 * This is the half a junction cannot work out for itself. Its own cell's scores are already reachable through
	 * the owning ANCellLevelInstance, but they say nothing about *which* of that cell's doorways leads inward —
	 * and the far cell frequently is not streamed in when INCellJunctionBeginPlay fires, so resolving it at
	 * runtime is not an option. Compare against the owning cell's score to get direction: lower leads toward the
	 * route or the landmark, higher leads away.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 ConnectedHotPathShortestScore = NEXUS::WorldAssembly::Proximity::UnreachableScore;

	/** As ConnectedHotPathShortestScore, measured against the sequential hot path. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 ConnectedHotPathSequentialScore = NEXUS::WorldAssembly::Proximity::UnreachableScore;

	/** As ConnectedHotPathShortestScore, measured against the nearest Important-flagged cell. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 ConnectedImportanceScore = NEXUS::WorldAssembly::Proximity::UnreachableScore;

	/**
	 * @return true if the cell across this junction is itself on the shortest-path hot path.
	 * @note Derived rather than stored: proximity scoring seeds `0` from exactly the cells carrying the flag, and
	 *       nothing else can reach `0` — every other cell is at least one hop from a seed. Distinct from
	 *       bHotPathShortest, which is true only when *both* cells are on the route: this being true while that is
	 *       false is precisely a doorway leading onto the route from off it.
	 */
	bool IsConnectedOnHotPathShortest() const { return ConnectedHotPathShortestScore == 0; }

	/** @return true if the cell across this junction is itself on the sequential hot path. See IsConnectedOnHotPathShortest. */
	bool IsConnectedOnHotPathSequential() const { return ConnectedHotPathSequentialScore == 0; }

	/** @return true if the cell across this junction is itself flagged Important. */
	bool IsConnectedImportant() const { return ConnectedImportanceScore == 0; }

	FString ToString() const
	{
		return FString::Printf(
			TEXT("[%d:%d] Connected: %s > [%d:%d] | HotShort: %s | HotSeq: %s | Connector: %s(%d) | Far Scores: %d/%d/%d"),
			NodeIdentifier,
			JunctionInstanceIdentifier,
			bConnected ? TEXT("True") : TEXT("False"),
			ConnectedNodeIdentifier,
			ConnectedJunctionInstanceIdentifier,
			bHotPathShortest ? TEXT("True") : TEXT("False"),
			bHotPathSequential ? TEXT("True") : TEXT("False"),
			bConnector ? TEXT("True") : TEXT("False"),
			ConnectorIdentifier,
			ConnectedHotPathShortestScore,
			ConnectedHotPathSequentialScore,
			ConnectedImportanceScore
		);
	}
};
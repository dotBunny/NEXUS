// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NWorldAssemblyMinimal.h"
#include "Collections/NGameplayTagCounter.h"
#include "NCellJunctionDetails.h"
#include "NCellLinkDetails.h"
#include "NCellAssemblyData.generated.h"

/**
 * Per-cell assembly metadata recorded on a generated cell, identifying the operation and graph node that produced it.
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/types/cell-assembly-data/">FNCellAssemblyData</a>
 */
USTRUCT(BlueprintType)
struct FNCellAssemblyData
{
	GENERATED_BODY()

	/**
	 * Reported by every proximity score when no seed of that kind is reachable from the cell — and equally when
	 * one sits further away than a byte can express. The two cases are deliberately indistinguishable: past a
	 * couple of hundred cells the difference has no bearing on anything that reads a proximity score, and
	 * collapsing them is what lets the scores ride replication as bytes rather than words.
	 * @note Aliases the shared constant so FNCellLinkDetails can name the same value; this is the spelling to
	 *       prefer wherever the assembly data is already in scope.
	 */
	static constexpr uint8 UnreachableScore = NEXUS::WorldAssembly::Proximity::UnreachableScore;

	/** Ticket of the operation that generated this cell. */
	UPROPERTY(VisibleInstanceOnly)
	int32 OperationTicket = 0;

	/** Identifier of the assembly graph node this cell was generated from. */
	UPROPERTY(VisibleInstanceOnly)
	int32 NodeIdentifier = 0;

	/** Seed used to generate this cell. */
	UPROPERTY(VisibleInstanceOnly)
	uint64 Seed = 0;

	/** Is this cell on the shortest-path hot path (spokes from start)? */
	UPROPERTY(VisibleInstanceOnly)
	bool bHotPathShortest = false;

	/** Is this cell on the sequential hot path (nearest-first visiting chain)? */
	UPROPERTY(VisibleInstanceOnly)
	bool bHotPathSequential = false;

	/** Assembly tags applied to this cell during the assembly operation. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	FGameplayTagContainer AssemblyTags;

	/** The final ContextTags for the Assembly Operation. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	FGameplayTagContainer ContextTags;

	/** The ContextTags this cell added. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	FGameplayTagContainer ContextTagsAdded;

	/** The final TagCounter */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	TArray<FNGameplayTagCount> TagCounter;

	/** World-space details for every junction on this cell, captured from the source graph node. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	TArray<FNCellJunctionDetails> JunctionDetails;

	/** Per-junction connection state (what each junction linked to, if anything) generated alongside JunctionDetails. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	TArray<FNCellLinkDetails> LinkDetails;

	/**
	 * Hops, counted in cells, from this cell to the nearest one on the shortest-path hot path; 0 when this cell is
	 * itself on it, UnreachableScore when no such cell is reachable (which is every cell when the assembly has no
	 * hot path at all).
	 */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	uint8 HotPathShortestScore = UnreachableScore;

	/** As HotPathShortestScore, measured against the sequential hot path instead. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	uint8 HotPathSequentialScore = UnreachableScore;

	/**
	 * Hops, counted in cells, from this cell to the nearest cell tagged NEXUS.WorldAssembly.Flag.Important; 0 when
	 * this cell carries the tag itself, UnreachableScore when none is reachable.
	 */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	uint8 ImportanceScore = UnreachableScore;
};

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameplayTagContainer.h"
#include "NTissueTagGroups.h"
#include "Collections/NGameplayTagCounter.h"
#include "Collections/NGameplayTagCounterConstraint.h"
#include "Engine/DataAsset.h"
#include "NTissue.generated.h"

class UNCell;

/**
 * A single cell entry within a UNTissue palette, carrying the generation constraints
 * (counts, weighting, graph position rules) that control how often and where the cell
 * is allowed to appear in the generated FNAssemblyGraph.
 */
USTRUCT()
struct NEXUSWORLDASSEMBLY_API FNTissueEntry
{
	GENERATED_BODY()
	
	/** Tags used to define behavior during the assembly process. */
	UPROPERTY(EditAnywhere, DisplayName="Assembly Tags", Category="Tagging",  meta = (Categories="NEXUS.WorldAssembly", ToolTip="Tags used to define behavior during the assembly process."))
	FGameplayTagContainer AssemblyTags;

	/** Tags added to the Context Tags collection moving forward for the graph, and future phases. */
	UPROPERTY(EditAnywhere, DisplayName="Added Context Tags", Category="Tagging", meta = (ToolTip="Tags added to the Context Tags collection moving forward for the graph, and future phases."))
	FGameplayTagContainer AddedContextTags;
	
	/** Tags required to be found in Context Tags for allowance to place this cell. **/
	UPROPERTY(EditAnywhere, DisplayName="Required Context Tags", Category="Tagging", meta = (ToolTip="Tags required to be found in Context Tags for allowance to place this cell."))
	FGameplayTagContainer RequiredContextTags;
	
	UPROPERTY(EditAnywhere, DisplayName="Tag Counter Constraints", Category="Tagging", meta = (ToolTip="TagCounter constraints that must pass for this cell to be included in cell selection."))
	TArray<FNGameplayTagCounterConstraint> TagCounterConstraints;
	
	UPROPERTY(EditAnywhere, DisplayName="Tag Counter Operations", Category="Tagging", meta = (ToolTip="Operations to apply to the TagCounter if the cell is placed/used."))
	TArray<FNGameplayTagCounterOperation> TagCounterOperations;
	
	/**
	 * DOES NOT DO ANYTHING CURRENTLY - Only used to determine Unique early out (1:1).
	 * A minimum number of times this cell must be used in the generated FNAssemblyGraph.
	 * @note A value of -1 indicates no minimum constraint.
	 */
	UPROPERTY(EditAnywhere)
	int32 MinimumCount = -1;

	/**
	 * The maximum number of times this cell can be used in the generated FNAssemblyGraph.
	 * @note A value of -1 indicates no maximum constraint.
	 */
	UPROPERTY(EditAnywhere)
	int32 MaximumCount = -1;
	
	/**
	 * The minimum number of cell links away this cell must be to be used again.
	 * @note A value of 0 indicates no constraint.
	 */
	UPROPERTY(EditAnywhere,meta=(ClampMin=0, UIMin=0))
	int32 MinimumNodeDistance = 1;
	
	/**
	* The minimum number of cell hops away from the start cell before this cell may be used.
	* The start cell is hop 0, its direct neighbours hop 1, etc. A value of N first allows the cell N hops out.
	* @note A value of 0 indicates no constraint.
	*/
	UPROPERTY(EditAnywhere,meta=(ClampMin=0, UIMin=0))
	int32 MinimumNodeDepth = 0;
	
	/** 
	 * Relative weight for random selection during generation.
	 * @note Higher values increase the probability of this cell being chosen.
	 */
	UPROPERTY(EditAnywhere)
	int32 Weighting = 1;
	
	/** The cell asset this entry refers to. */
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UNCell> Cell;
};

/**
 * A collection of NCells
 * @remark Can think of this as a definition of multiple cell types used with tissue.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Tissue",
	meta = (DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/tissue"))
class NEXUSWORLDASSEMBLY_API UNTissue : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Flatten a tissue (and any referenced AdditionalTissue) into a single cell-to-entry map.
	 * @param Tissue The root tissue to start from.
	 * @param OutCellMap Populated with every cell reachable from Tissue and the effective entry to use.
	 * @param OutTagGroups Accumulated tag groups from all visited tissues.
	 * @param OutProcessedSets Tracks tissue assets already visited so cycles do not cause infinite recursion.
	 */
	static void BuildTissueMap(UNTissue* Tissue, TMap<TObjectPtr<UNCell>, FNTissueEntry>& OutCellMap, 
		FNTissueTagGroups& OutTagGroups, 
		TArray<UNTissue*>& OutProcessedSets);
	
	/** Tag groups that drive placement behavior for this tissue's cells. */
	UPROPERTY(EditAnywhere, DisplayName="Assembly Tag Groups")
	FNTissueTagGroups TagGroups = FNTissueTagGroups();
	
	/** The cells that directly belong to this tissue, along with per-cell generation constraints. */
	UPROPERTY(EditAnywhere, meta=(TitleProperty="{Cell}"))
	TArray<FNTissueEntry> Cells;

	/** Additional tissue assets merged into this one during BuildTissueMap. */
	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UNTissue>> AdditionalTissue;
};
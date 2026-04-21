// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCellJunctionDetails.h"
#include "NCellRootDetails.h"
#include "NCell.generated.h"

/**
 * The data-only representation of a NCellActor and its details.
 * @note The UNCell is effectively a side-car file to NCellActor.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Cell")
class NEXUSPROCGEN_API UNCell : public UDataAsset
{
	friend class FNProcGenEditorUtils;
	
	GENERATED_BODY()

public:
	/** The level asset this cell represents; streamed in when the cell is placed at runtime. */
	UPROPERTY(VisibleAnywhere, DisplayName="Level")
	TSoftObjectPtr<UWorld> World;

	/** Root-level metadata captured from the cell's NCellRootComponent (bounds, hull, voxel data). */
	UPROPERTY(VisibleAnywhere)
	FNCellRootDetails Root;

	/** Per-junction metadata keyed by junction identifier; populated from NCellJunctionComponents found in the level. */
	UPROPERTY(VisibleAnywhere)
	TMap<int32, FNCellJunctionDetails> Junctions;

private:
	/** Schema version stored alongside the asset so older data can be upgraded after reloads. */
	UPROPERTY(VisibleAnywhere)
	int Version = 0;
};
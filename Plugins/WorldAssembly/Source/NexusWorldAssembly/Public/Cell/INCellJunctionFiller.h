// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "INCellJunctionFiller.generated.h"

UINTERFACE()
class NEXUSWORLDASSEMBLY_API UNCellJunctionFiller : public UInterface
{
	GENERATED_BODY()
};


/**
 * Interface implemented by actors spawned to fill an unconnected junction during world assembly.
 *
 * When a junction is left unconnected after generation, UNCellJunctionComponent::Fill selects an eligible filler entry
 * (or falls back to the project-wide default filler) and spawns it at the junction. If the spawned actor implements
 * this interface, OnInitializedFromJunction is invoked so the filler can read its placement context — the owning cell's
 * level instance and the junction it was spawned for.
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/types/cell-junction-filler/">INCellJunctionFiller</a>
 */
class NEXUSWORLDASSEMBLY_API INCellJunctionFiller
{
	GENERATED_BODY()
public:
	/**
	 * Called once this actor has been spawned to fill an unconnected junction.
	 * @param CellLevelInstance The level instance of the cell that owns the junction, providing post-assembly context.
	 * @param JunctionComponent The junction component that spawned and owns this filler.
	 * @param JunctionIndex The persisted instance identifier of the junction this filler was spawned for.
	 */
	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category="NEXUS|World Assembly")
	void OnInitializedFromJunction(ANCellLevelInstance* CellLevelInstance, UNCellJunctionComponent* JunctionComponent,  int32 JunctionIndex);
};
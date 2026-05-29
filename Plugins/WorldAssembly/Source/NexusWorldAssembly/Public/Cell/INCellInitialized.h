// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellLevelInstance.h"
#include "INCellInitialized.generated.h"

UINTERFACE()
class NEXUSWORLDASSEMBLY_API UNCellInitialized : public UInterface
{
	GENERATED_BODY()
};


/**
 * Interface implemented by actors that need to react when their owning cell has been initialized from a proxy.
 *
 * Actors placed within a cell level that implement this interface are discovered and registered as initialize-callback
 * actors. When the ANCellActor finishes applying data from its proxy via InitializeFromProxy, it invokes
 * OnInitializedFromProxy on each registered actor, handing over the spawned ANCellLevelInstance. This is the entry
 * point for gameplay actors to read post-assembly context (for example the accumulated OutputTags) from the cell.
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/types/cell-initialized/">INCellInitialized</a>
 */
class NEXUSWORLDASSEMBLY_API INCellInitialized
{
	GENERATED_BODY()
public:
	/**
	 * Called once the owning cell has been initialized from its proxy.
	 * @param CellLevelInstance The level instance the cell was initialized from, providing post-assembly context.
	 */
	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category="NEXUS|World Assembly")
	void OnInitializedFromProxy(ANCellLevelInstance* CellLevelInstance);
};
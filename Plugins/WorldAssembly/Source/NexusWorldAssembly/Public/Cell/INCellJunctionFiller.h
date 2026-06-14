// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellLevelInstance.h"
#include "INCellJunctionFiller.generated.h"

UINTERFACE()
class NEXUSWORLDASSEMBLY_API UNCellJunctionFiller : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implement on an actor to react to being spawned as a junction filler during World Assembly generation.
 *
 * Actors assigned to a junction's Fillers (or the project-wide Default Filler) must implement this interface; the
 * junction calls OnInitializedFromProxy on the freshly spawned actor so it can size or configure itself from the
 * junction it fills.
 */
class NEXUSWORLDASSEMBLY_API INCellJunctionFiller
{
	GENERATED_BODY()
public:

	/**
	 * Called on a freshly spawned filler actor immediately after it is placed at a junction, letting it size or
	 * configure itself from the junction it fills.
	 * @param CellLevelInstance The cell level instance that owns the junction being filled.
	 * @param JunctionComponent The junction this actor was spawned to fill.
	 * @param JunctionIndex The filled junction's link instance identifier (LinkDetails.JunctionInstanceIdentifier).
	 */
	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category="NEXUS|World Assembly")
	void OnInitializedFromJunction(ANCellLevelInstance* CellLevelInstance, UNCellJunctionComponent* JunctionComponent,  int32 JunctionIndex);
};
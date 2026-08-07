// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellJunctionComponent.h"
#include "NCellJunctionConnection.h"
#include "INCellJunctionConnector.generated.h"

/** Reflection marker for the INCellJunctionConnector interface; INCellJunctionConnector carries the contract implementors provide. */
UINTERFACE()
class NEXUSWORLDASSEMBLY_API UNCellJunctionConnector : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implemented by actors that bridge two junctions the connector pass paired across open space.
 *
 * Unlike a junction filler, which caps a single opening, a connector spans two — so it is spawned once per pairing
 * rather than once per junction, and only after both cells have streamed in.
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/types/cell-junction-connector/">INCellJunctionConnector</a>
 */
class NEXUSWORLDASSEMBLY_API INCellJunctionConnector
{
	GENERATED_BODY()

public:

	/**
	 * Called once both ends of a pairing are live, to build the geometry that joins them.
	 * @param StartCellLevelInstance The cell owning the start junction.
	 * @param StartJunctionComponent The junction the route leaves from.
	 * @param StartJunctionIndex Instance identifier of the start junction.
	 * @param EndCellLevelInstance The cell owning the end junction.
	 * @param EndJunctionComponent The junction the route arrives at.
	 * @param EndJunctionIndex Instance identifier of the end junction.
	 * @param Path The route the connector pass proved clear: a center curve plus the four socket-corner curves that
	 *        bound it. Geometry lofted through the corner curves is guaranteed to fit the space that was tested.
	 * @note Which end is Start is decided by the connector pass, not by proximity or spawn order; it is the same end
	 *       whose authored connector overrides won the selection that produced this actor.
	 */
	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category="NEXUS|World Assembly")
	void OnConnectJunctions(
		ANCellLevelInstance* StartCellLevelInstance, UNCellJunctionComponent* StartJunctionComponent,  int32 StartJunctionIndex,
		ANCellLevelInstance* EndCellLevelInstance, UNCellJunctionComponent* EndJunctionComponent,  int32 EndJunctionIndex,
		const FNCellJunctionConnectorPath& Path);
};

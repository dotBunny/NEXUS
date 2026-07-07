// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellLinkDetails.h"
#include "INCellJunctionBeginPlay.generated.h"

UINTERFACE()
class NEXUSWORLDASSEMBLY_API UNCellJunctionBeginPlay : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface implemented by actors that need to react when a cell junction begins play.
 *
 * Actors assigned to a junction's BeginPlay callback list (UNCellJunctionComponent::OnBeginPlayCallback) are notified
 * during the junction component's BeginPlay: for each assigned actor that implements this interface, OnJunctionBeginPlay
 * is invoked with the junction's resolved link details. This lets gameplay actors react to how the junction was wired up
 * during assembly — in particular whether it connected to another cell and which cell and junction it links to.
 */
class NEXUSWORLDASSEMBLY_API INCellJunctionBeginPlay
{
	GENERATED_BODY()

public:
	/**
	 * Called during the owning junction's BeginPlay for each actor registered on its BeginPlay callback list.
	 * @param Junction The cell junction component that is making the call.
	 * @param CellLinkDetails The junction's resolved connection state, including whether it connected to another cell and the connected node and junction identifiers.
	 */
	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category="NEXUS|World Assembly")
	void OnJunctionBeginPlay(UNCellJunctionComponent* Junction, FNCellLinkDetails CellLinkDetails);
};
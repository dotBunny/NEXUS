// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"
#include "Organ/NOrganVolume.h"

class UNCell;

/**
 * Flags describing what categories of ProcGen objects are in the current editor selection.
 */
UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENProcGenSelectionFlags : uint8
{
	None = 0,
	CellActor = 1 << 0,
	OrganVolume = 1 << 1
};
ENUM_CLASS_FLAGS(ENProcGenSelectionFlags)

/**
 * Editor-only helpers for inspecting the current selection/world and persisting cell side-car data.
 */
class NEXUSPROCGENEDITOR_API FNProcGenEditorUtils
{
public:
	/** @return true if the currently-edited world contains at least one ANCellActor. */
	static bool IsCellActorPresentInCurrentWorld();

	/** @return First ANCellActor found in the currently-edited world, or nullptr. */
	static ANCellActor* GetCellActorFromCurrentWorld();

	/** @return true if the editor selection contains at least one ANCellActor. */
	static bool IsCellActorSelected();

	/** @return Every ANCellActor in the current editor selection. */
	static TArray<ANCellActor*> GetSelectedCellActors();

	/** @return true if the editor selection contains at least one ANOrganVolume. */
	static bool IsOrganVolumeSelected();

	/** @return true if the currently-edited world contains at least one UNOrganComponent. */
	static bool IsOrganComponentPresentInCurrentWorld();

	/** @return Every ANOrganVolume in the current editor selection. */
	static TArray<ANOrganVolume*> GetSelectedOrganVolumes();

	/** @return Every UNOrganComponent from the current editor selection (volumes and standalone components). */
	static TArray<UNOrganComponent*> GetSelectedOrganComponents();

	/** @return Bitfield describing which ProcGen categories the current selection includes. */
	static ENProcGenSelectionFlags GetSelectionFlags();

	/** @return true if there are cell proxies currently alive from the last generation pass. */
	static bool HasGeneratedCellProxies();

	/** @return true if any selected actor corresponds to a generated cell proxy. */
	static bool HasSelectedGeneratedCellProxies();

	/**
	 * Persist the cell's side-car asset to disk.
	 * @param CellActor Specific cell actor to save, or nullptr to save the world's primary cell actor.
	 */
	static void SaveCell(UWorld* World, ANCellActor* CellActor = nullptr);

	/**
	 * Refresh the cell asset from the supplied cell actor.
	 * @return true if Cell was updated and marked dirty, false if there was nothing to copy.
	 */
	static bool UpdateCell(UNCell* Cell, ANCellActor* CellActor);

	/** @return true if the actor participates in generated cell data (has a cell-root or junction component). */
	FORCEINLINE static bool EffectsGeneratedData(const AActor* ContextActor)
	{
		if (ContextActor == nullptr) return false;
		if (ContextActor->FindComponentByClass<UNCellRootComponent>() != nullptr || ContextActor->FindComponentByClass<UNCellJunctionComponent>() != nullptr) return true;

		return false;
	}

	/** @return true if the component is a cell-root or junction component. */
	FORCEINLINE static bool EffectsGeneratedData(const UActorComponent* ContextActorComponent)
	{
		if (ContextActorComponent == nullptr) return false;
		return ContextActorComponent->IsA<UNCellRootComponent>() || ContextActorComponent->IsA<UNCellJunctionComponent>();
	}
};
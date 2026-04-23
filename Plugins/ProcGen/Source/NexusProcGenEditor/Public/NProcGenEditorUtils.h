// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"
#include "Developer/NDebugActor.h"
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

	/**
	 * Spawns a single transient debug actor into World that visualizes the simple-collision geometry of every actor
	 * whose bounds fall inside any of the supplied containment volumes.
	 *
	 * Pulls the world-actor list via FNActorUtils::GetWorldActors using the ProcGen filter settings, extracts each
	 * actor's simple-collision representation with FNRawMeshFactory::FromActorsInBounds, then merges every emitted
	 * mesh into one ANDebugActor via FNRawMeshUtils::CreateRawMeshVisualizers (single-actor mode). The visualizer is
	 * shaded with UNProcGenEditorSettings::CollisionVisualizerMaterial.
	 *
	 * @param World World to iterate for collision sources and to spawn the visualizer actor into. Must be valid.
	 * @param Bounds Containment volumes; only actors whose bounds fit inside at least one are visualized.
	 * @return The merged visualizer actor. Editor-only / diagnostic — do not use in shipping content.
	 * @note Performs a synchronous load of the configured visualizer material.
	 */
	static ANDebugActor* CreateWorldCollisionVisualizerActor(UWorld* World, const TArray<FBoxSphereBounds>& Bounds);

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
	
	/** @return First ANCellActor found in the currently-edited world, or nullptr. */
	static ANCellActor* GetCellActorFromCurrentWorld();
	
	/** @return Every ANCellActor in the current editor selection. */
	static TArray<ANCellActor*> GetSelectedCellActors();
	
	/** @return Every UNOrganComponent from the current editor selection (volumes and standalone components). */
	static TArray<UNOrganComponent*> GetSelectedOrganComponents();
	
	/** @return Every ANOrganVolume in the current editor selection. */
	static TArray<ANOrganVolume*> GetSelectedOrganVolumes();

	/** @return Bitfield describing which ProcGen categories the current selection includes. */
	static ENProcGenSelectionFlags GetSelectionFlags();

	/** @return true if there are cell proxies currently alive from the last generation pass. */
	static bool HasGeneratedCellProxies();

	/** @return true if any selected actor corresponds to a generated cell proxy. */
	static bool HasSelectedGeneratedCellProxies();
	
	/** @return true if the currently-edited world contains at least one ANCellActor. */
	static bool IsCellActorPresentInCurrentWorld();

	/** @return true if the editor selection contains at least one ANCellActor. */
	static bool IsCellActorSelected();

	/** @return true if the currently-edited world contains at least one UNOrganComponent. */
	static bool IsOrganComponentPresentInCurrentWorld();

	/** @return true if the editor selection contains at least one ANOrganVolume. */
	static bool IsOrganVolumeSelected();

	/**
	 * Refreshes the cell side-car asset from CellActor (via UpdateCell), marks the package dirty, and writes it to disk.
	 *
	 * Resolves or creates the UNCell package via UAssetDefinition_NCell::GetOrCreatePackage, then commits the result with
	 * UEditorAssetLibrary::SaveLoadedAsset. When CellActor is null, the world's primary ANCellActor is resolved via
	 * FNProcGenUtils::GetCellActorFromWorld; if no cell actor can be found at all, a warning is logged and the call no-ops.
	 *
	 * @param World World whose cell asset should be persisted. Used both to resolve the package and (when CellActor is null) to find the cell actor.
	 * @param CellActor Specific cell actor to save, or nullptr to use the world's primary cell actor.
	 * @note Editor-only. Triggers the same slow-task UI as UpdateCell (it's invoked internally) and performs synchronous disk I/O.
	 */
	static void SaveCell(UWorld* World, ANCellActor* CellActor = nullptr);

	/**
	 * Refresh the cell asset from the supplied cell actor.
	 * @return true if Cell was updated and marked dirty, false if there was nothing to copy.
	 */
	static bool UpdateCell(UNCell* Cell, ANCellActor* CellActor);
};
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"
#include "Developer/NDebugActor.h"
#include "Organ/NOrganVolume.h"

class UNCell;

/**
 * Flags describing what categories of World Assembly objects are in the current editor selection.
 */
UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENWorldAssemblySelectionFlags : uint8
{
	None = 0,
	CellActor = 1,
	OrganVolume = 2
};
ENUM_CLASS_FLAGS(ENWorldAssemblySelectionFlags)

/**
 * Editor-only helpers for inspecting the current selection/world and persisting cell side-car data.
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEditorUtils
{
public:

	/**
	 * Builds — or refreshes in place — the single transient debug actor that visualizes the merged simple-collision
	 * geometry of every actor whose bounds fall inside any of the supplied containment volumes.
	 *
	 * Pulls the world-actor list via FNActorUtils::GetWorldActors using the World Assembly filter settings, extracts each
	 * actor's simple-collision representation with FNRawMeshFactory::FromActorsInBounds, and merges every emitted mesh
	 * into one piece of geometry shaded with UNWorldAssemblyEditorSettings::CollisionVisualizerMaterial.
	 *
	 * When ExistingActor is supplied, the merged geometry is swapped onto its dynamic-mesh component in place (no
	 * re-spawn, preserving actor identity and selection) — even when the merged mesh is empty, which clears the
	 * visualizer. When ExistingActor is null a fresh actor is spawned, but only if there is geometry to show and the
	 * visualizer material is configured.
	 *
	 * @param World World to iterate for collision sources and to spawn the visualizer actor into. Must be valid.
	 * @param Bounds Containment volumes; only actors whose bounds fit inside at least one are visualized.
	 * @param ExistingActor Live visualizer to refresh in place, or null to spawn a new one.
	 * @param OutSourceActors Receives every actor that passed the collision filter — the visualizer's source set, used
	 *        by callers to decide whether a later world change is relevant to the visualizer.
	 * @return The visualizer actor (ExistingActor when supplied; otherwise the newly spawned actor, or null when there
	 *         was nothing to show or no material). Editor-only / diagnostic — do not use in shipping content.
	 * @note Performs a synchronous load of the configured visualizer material.
	 */
	static ANDebugActor* RefreshWorldCollisionVisualizerActor(UWorld* World, const TArray<FBoxSphereBounds>& Bounds,
		ANDebugActor* ExistingActor, TArray<AActor*>& OutSourceActors);

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

	/**
	 * @param bSorted When true, results are returned in a stable sort order suitable for deterministic traversal.
	 * @return Every UNOrganComponent from the current editor selection (volumes and standalone components).
	 */
	static TArray<UNOrganComponent*> GetSelectedOrganComponents(bool bSorted = true);

	/**
	 * @param bSorted When true, results are returned in a stable sort order suitable for deterministic traversal.
	 * @return Every ANOrganVolume in the current editor selection.
	 */
	static TArray<ANOrganVolume*> GetSelectedOrganVolumes(bool bSorted = true);

	/** @return Bitfield describing which World Assembly categories the current selection includes. */
	static ENWorldAssemblySelectionFlags GetSelectionFlags();

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
	 * FNWorldAssemblyUtils::GetCellActorFromWorld; if no cell actor can be found at all, a warning is logged and the call no-ops.
	 *
	 * @param World World whose cell asset should be persisted. Used both to resolve the package and (when CellActor is null) to find the cell actor.
	 * @param CellActor Specific cell actor to save, or nullptr to use the world's primary cell actor.
	 * @note Editor-only. Triggers the same slow-task UI as UpdateCell (it's invoked internally) and performs synchronous disk I/O.
	 */
	static void SaveCell(UWorld* World, ANCellActor* CellActor = nullptr, bool bForceSave = false);

	/**
	 * Refreshes the cell side-car asset from CellActor (via UpdateCell) and marks the package dirty, but does NOT write
	 * it to disk. This is the in-memory half of SaveCell, split out so callers that run inside the world-save flow can
	 * sync the data here and defer the side-car's own SavePackage to PostSaveWorldWithContext (saving a package from
	 * inside PreSaveWorldWithContext is a re-entrant save and is unsafe).
	 *
	 * Resolves or creates the UNCell package via UAssetDefinition_NCell::GetOrCreatePackage. When CellActor is null, the
	 * world's primary ANCellActor is resolved via FNWorldAssemblyUtils::GetCellActorFromWorld; if no cell actor can be
	 * found at all, a warning is logged and the call no-ops.
	 *
	 * @param World World whose cell asset should be synced.
	 * @param CellActor Specific cell actor to sync, or nullptr to use the world's primary cell actor.
	 * @param bForceSave When true, flags the side-car for a flush even if UpdateCell reported no changes.
	 * @return The side-car UNCell that was dirtied and now needs a disk flush, or nullptr when nothing changed (or the
	 *         cell actor / package could not be resolved).
	 * @note Editor-only. Triggers the same slow-task UI as UpdateCell (it's invoked internally).
	 */
	static UNCell* SyncCell(UWorld* World, ANCellActor* CellActor = nullptr, bool bForceSave = false);

	/**
	 * Ensures the callback actors required by the cell exist and are initialized in the world.
	 * @param World The world containing the cell.
	 * @param CellActor The cell actor whose callback actors should be validated.
	 */
	static void EnsureCellInitializedCallbackActors(const UWorld* World, ANCellActor* CellActor);

	/**
	 * Refresh the cell asset from the supplied cell actor.
	 * @return true if Cell was updated and marked dirty, false if there was nothing to copy.
	 */
	static bool UpdateCell(UNCell* Cell, ANCellActor* CellActor);

	/** @return true if the currently-selected organ can be generated. */
	static bool CanGenerateSelectedOrgan();

	/** @return true if every organ in the current level can be generated. */
	static bool CanGenerateAllOrgans();

	/**
	 * Gather the asset data for every UNCell (and subclass) known to the asset registry.
	 * @param bWaitForFullScan when true, blocks until the entire asset registry has been scanned before gathering.
	 * @return the asset data for all discovered cell data assets.
	 * @note Pass true only from commandlets or other headless contexts; the synchronous scan will hard-stall an interactive editor on large projects.
	 */
	static TArray<FAssetData> GetAllCellDataAssetData(bool bWaitForFullScan);
};
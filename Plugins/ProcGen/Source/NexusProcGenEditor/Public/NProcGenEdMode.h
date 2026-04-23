// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorModeManager.h"
#include "EdMode.h"
#include "Cell/NCellActor.h"
#include "Developer/NDebugActor.h"

class FNProcGenOperationContext;
class UNProcGenOperation;

/**
 * Custom editor mode for ProcGen cell/organ authoring.
 *
 * Provides viewport overlays (bounds/hull/voxel) for the focused cell actor, orchestrates the
 * editor-side preview generation for the selected organ, and surfaces auto-generate warnings
 * to the user via on-screen HUD messages.
 */
class FNProcGenEdMode final : public FEdMode
{
public:
	/**
	 * Which side-car slot the cell-editor view is currently focused on.
	 */
	enum class ENCellEdMode : uint8
	{
		Bounds = 0,
		Hull = 1,
		Voxel = 2
	};

	/**
	 * Visualization style for voxel overlays.
	 */
	enum class ENCellVoxelMode : uint8
	{
		None = 0,
		Grid = 1,
		Points = 2
	};

	/** @return Cached axis-aligned bounds for the focused cell. */
	static FBox GetCachedBounds() { return CachedBounds; }

	/** @return Cached bounds overlay color. */
	static const FLinearColor& GetCachedBoundsColor() { return CachedBoundsColor; }

	/** @return Cached bounds overlay vertex positions. */
	static const TArray<FVector>& GetCachedBoundsVertices() { return CachedBoundsVertices; }

	/** @return Cached hull overlay color. */
	static const FLinearColor& GetCachedHullColor() { return CachedHullColor; }

	/** @return Cached voxel overlay data. */
	static const FNCellVoxelData& GetCachedVoxelData() { return CachedVoxelData; }

	/** @return Cached hull overlay vertex positions. */
	static const TArray<FVector>& GetCachedHullVertices() { return CachedHullVertices; }

	/** @return The cell actor this mode is currently focused on. */
	static ANCellActor* GetCellActor() { return CellActor; }

	/** @return The currently active cell-edit sub-mode. */
	static ENCellEdMode GetCellEdMode() { return CellEdMode; }

	/** Force the edit mode back to a safe state if the current one becomes invalid. */
	static void ProtectCellEdMode();

	/** @return The currently active voxel visualization style. */
	static ENCellVoxelMode GetCellVoxelMode() { return CellVoxelMode; }

	/** Set the voxel visualization style. */
	static void SetCellVoxelMode(const ENCellVoxelMode InCellVoxelMode) { CellVoxelMode = InCellVoxelMode; }

	/** @return true if a cell actor is currently focused. */
	static bool HasCellActor() { return CellActor != nullptr; }

	/** @return true if no cell actor is currently focused. */
	static bool HasNoCellActor() { return CellActor == nullptr; }

	/** @return true if this editor mode is currently the active level-editor mode. */
	static bool IsActive() { return GLevelEditorModeTools().IsModeActive(Identifier); }

	/** @return true if this editor mode is not the active level-editor mode. */
	static bool IsNotActive() { return !GLevelEditorModeTools().IsModeActive(Identifier); }

	/** Set the active cell-edit sub-mode. */
	static void SetCellEdMode(const ENCellEdMode InCellEdMode) { CellEdMode = InCellEdMode; }

	/** @return true when a world-collision visualizer actor is currently alive for the focused world. */
	static bool HasCollisionVisualizer() { return CollisionVisualizer != nullptr; }
	
	static void OnActorDeleted(AActor* Actor);
	
	/**
	 * Spawns the world-collision visualizer — a single merged ANDebugActor whose mesh is the union of the
	 * simple-collision geometry of every world actor that passes the ProcGen world-actor filter
	 * (FNProcGenOperationContext::CreateWorldActorFilterSettings), shaded with
	 * UNProcGenEditorSettings::CollisionVisualizerMaterial — and caches it on the mode. Any previously-spawned
	 * visualizer is destroyed first, so this doubles as a refresh. Editor-only / diagnostic; the actor is
	 * transient and will not be saved with the level.
	 * @param World World to iterate for collision sources and to spawn the visualizer into. Must be valid.
	 * @return The newly cached visualizer actor, or nullptr if no collision geometry was extracted.
	 */
	static TObjectPtr<ANDebugActor> CreateCollisionVisualizer(UWorld* World);
	/**
	 * Destroys the cached world-collision visualizer actor if one exists. No-op when none is alive. Called automatically
	 * from CreateCollisionVisualizer (refresh) and during Exit() so the visualizer doesn't outlive the editor mode.
	 */
	static void DestroyCollisionVisualizer();
	
	/** Unique identifier registered with the editor-mode manager. */
	const static FEditorModeID Identifier;

	/** HUD message shown when focused cell data is dirty and needs re-saving. */
	const static FText DirtyMessage;

	/** HUD message shown when both auto-bounds and auto-hull are disabled. */
	const static FText AutoBoundsHullMessage;

	/** HUD message shown when auto-bounds is disabled. */
	const static FText AutoBoundsMessage;

	/** HUD message shown when auto-hull is disabled. */
	const static FText AutoHullMessage;

	/** HUD message shown when auto-voxel is disabled. */
	const static FText AutoVoxelMessage;

	virtual ~FNProcGenEdMode() override;

	//~FEdMode
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
	//End FEdMode

private:
	/** Pixel spacing between stacked HUD messages. */
	const int MessageSpacing = 20;

	static TArray<FVector> CachedHullVertices;
	static FLinearColor CachedHullColor;
	static FBox CachedBounds;
	static FNCellVoxelData CachedVoxelData;
	static FLinearColor CachedBoundsColor;
	static TArray<FVector> CachedBoundsVertices;
	static ANCellActor* CellActor;
	static ENCellEdMode CellEdMode;
	static ENCellVoxelMode CellVoxelMode;
	static TObjectPtr<ANDebugActor> CollisionVisualizer;

	/** Editor-side operation used to preview organ generation. */
	TObjectPtr<UNProcGenOperation> OrganGenerator;

	/** Gate that lets Tick skip work when nothing relevant has changed. */
	bool bCanTick = false;

	/** True if any focused cell actor currently has unsaved dirty state. */
	bool bHasDirtyActors = false;

	/** User-disabled auto-bounds regeneration; surfaces a HUD warning. */
	bool bAutoBoundsDisabled = false;

	/** User-disabled auto-hull regeneration; surfaces a HUD warning. */
	bool bAutoHullDisabled = false;

	/** User-disabled auto-voxel regeneration; surfaces a HUD warning. */
	bool bAutoVoxelDisabled = false;

	/** Hash of the previously-selected organ set, used to detect selection changes per tick. */
	uint32 PreviousSelectedOrganHash = 0;
	
	FDelegateHandle OnLevelActorDeletedHandle;
};

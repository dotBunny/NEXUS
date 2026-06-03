// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorModeManager.h"
#include "EdMode.h"
#include "NWorldAssemblyEditorModule.h"
#include "CanvasItems/NMultiLineTextBoxCanvasItem.h"
#include "Cell/NCellActor.h"
#include "Developer/NDebugActor.h"
#include "UObject/ObjectKey.h"
#include "Visualizers/NCellRootComponentVisualizer.h"

class FNAssemblyOperationContext;
class UNAssemblyOperation;
struct FPropertyChangedEvent;

enum class ENWorldAssemblyEdModeRenderMode
{
	All,
	CellScreenshot,
	LevelScreenshot,
	None,
};

/**
 * Custom editor mode for World Assembly cell/organ authoring.
 *
 * Provides viewport overlays (bounds/hull/voxel) for the focused cell actor, orchestrates the
 * editor-side preview generation for the selected organ, and surfaces auto-generate warnings
 * to the user via on-screen HUD messages.
 */
class FNWorldAssemblyEdMode final : public FEdMode
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
	
	/** @return Cached hull overlay vertex positions. */
	static const TArray<FIntVector2>& GetCachedHullEdges() { return CachedHullEdges; }

	/** @return The cell actor this mode is currently focused on, or nullptr if none or stale. */
	static ANCellActor* GetCellActor() { return CellActor.Get(); }

	/** @return The currently active cell-edit sub-mode. */
	static ENCellEdMode GetCellEdMode() { return CellEdMode; }

	/** Force the edit mode back to a safe state if the current one becomes invalid. */
	static void ProtectCellEdMode();

	/** @return The currently active voxel visualization style. */
	static ENCellVoxelMode GetCellVoxelMode() { return CellVoxelMode; }

	/** Set the voxel visualization style. */
	static void SetCellVoxelMode(const ENCellVoxelMode InCellVoxelMode) { CellVoxelMode = InCellVoxelMode; }

	/** @return true if a cell actor is currently focused. */
	static bool HasCellActor() { return CellActor.IsValid(); }

	/** @return true if no cell actor is currently focused. */
	static bool HasNoCellActor() { return !CellActor.IsValid(); }

	/** @return true if this editor mode is currently the active level-editor mode. */
	static bool IsActive() { return GLevelEditorModeTools().IsModeActive(Identifier); }

	/** @return true if this editor mode is not the active level-editor mode. */
	static bool IsNotActive() { return !GLevelEditorModeTools().IsModeActive(Identifier); }

	/** Set the active cell-edit sub-mode. */
	static void SetCellEdMode(const ENCellEdMode InCellEdMode)
	{
		if (CellEdMode == InCellEdMode) return;
		
		FNWorldAssemblyEditorModule& Module = FNWorldAssemblyEditorModule::Get();
		Module.RootComponentVisualizer->ClearSelection();
		
		CellEdMode = InCellEdMode;
	}

	/** @return true when a world-collision visualizer actor is currently alive for the focused world. */
	static bool HasCollisionVisualizer() { return CollisionVisualizer != nullptr; }
	
	static void OnActorDeleted(AActor* Actor);
	
	/** Set the active render mode used to draw World Assembly debug geometry in the edit mode. */
	static void SetRenderMode(const ENWorldAssemblyEdModeRenderMode Mode) { RenderMode = Mode; }
	
	/**
	 * Builds — or refreshes in place — the world-collision visualizer: a single merged ANDebugActor whose mesh is the
	 * union of the simple-collision geometry of every world actor that passes the World Assembly world-actor filter
	 * (FNCreateVirtualWorldTask::CreateWorldActorFilterSettings), shaded with
	 * UNWorldAssemblyEditorSettings::CollisionVisualizerMaterial, and caches it on the mode.
	 *
	 * When no visualizer is cached this spawns one and starts listening for world changes; when one already exists its
	 * geometry is swapped in place (preserving actor identity and selection). The cached set of source actors is
	 * refreshed and the dirty flag cleared each call. Editor-only / diagnostic; the actor is transient and will not be
	 * saved with the level.
	 * @param World World to iterate for collision sources and to spawn the visualizer into. Must be valid.
	 * @return The cached visualizer actor, or nullptr if none exists and no collision geometry was extracted.
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

	virtual ~FNWorldAssemblyEdMode() override;

	//~FEdMode
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
	//End FEdMode

private:
	static void RenderCellJunctionPenetrationDistance(FPrimitiveDrawInterface* PDI, const UNCellJunctionComponent* JunctionComponent, FVector2D SocketSize, float MatchingDepth = 0);

	/** Subscribe to the editor world-change delegates that drive live visualizer refreshes. Called when one is spawned. */
	static void BindWorldChangeDelegates();

	/** Unsubscribe from the editor world-change delegates. Called when the visualizer is destroyed or the mode exits. */
	static void UnbindWorldChangeDelegates();

	/** Flag the cached visualizer for a rebuild on the next Tick. Bursts of changes coalesce into a single rebuild. */
	static void MarkCollisionVisualizerDirty() { bCollisionVisualizerDirty = true; }

	/**
	 * @return true when a change to Actor could alter the cached visualizer — i.e. Actor currently passes the collision
	 *         filter, or it was part of the source set the live visualizer was last built from (so a delete / collision
	 *         toggle / ignore-tag still forces it to drop out). Always false while no visualizer is alive.
	 */
	static bool ShouldRebuildForActor(const AActor* Actor);

	/** @return The actor affected by a change delegate payload — the object itself, or its owner when it is a component. */
	static AActor* ResolveAffectedActor(UObject* Object);

	/** Delegate: a relevant actor was added to the level — flag a refresh. */
	static void OnLevelActorAdded(AActor* Actor);

	/** Delegate: a transform gizmo drag ended on Object — flag a refresh when it is relevant. */
	static void OnObjectMoved(UObject& Object);

	/** Delegate: a finalized (non-interactive) property edit landed on Object — flag a refresh when it is relevant. */
	static void OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);

	/** Delegate: an undo/redo transaction completed — geometry can't be cheaply diffed, so always flag a refresh. */
	static void OnUndoRedo();

	/** Pixel spacing between stacked HUD messages. */
	const int32 MessageSpacing = 20;

	static TArray<FVector> CachedHullVertices;
	static TArray<FIntVector2> CachedHullEdges;
	static FLinearColor CachedHullColor;
	static FBox CachedBounds;
	static FNCellVoxelData CachedVoxelData;
	static FLinearColor CachedBoundsColor;
	static TArray<FVector> CachedBoundsVertices;
	static TWeakObjectPtr<ANCellActor> CellActor;
	static ENCellEdMode CellEdMode;
	static ENCellVoxelMode CellVoxelMode;
	static TObjectPtr<ANDebugActor> CollisionVisualizer;
	static ENWorldAssemblyEdModeRenderMode RenderMode;

	/** Actors that sourced the geometry of the live visualizer; lets change delegates test relevance in O(1). */
	static TSet<FObjectKey> CollisionSourceActors;

	/** Set by the world-change delegates when the visualizer needs rebuilding; consumed (and cleared) in Tick. */
	static bool bCollisionVisualizerDirty;

	static FDelegateHandle OnLevelActorAddedHandle;
	static FDelegateHandle OnObjectMovedHandle;
	static FDelegateHandle OnObjectPropertyChangedHandle;
	static FDelegateHandle OnUndoRedoHandle;

	/** Editor-side operation used to preview organ generation. */
	TObjectPtr<UNAssemblyOperation> OrganGenerator;

	/** Gate that lets Tick skip work when nothing relevant has changed. */
	bool bCanTick = false;

	/** True if any focused cell actor currently has unsaved dirty state. */
	bool bHasDirtyActors = false;

	/** User-disabled auto-bounds regeneration; surfaces a HUD warning. */
	bool bAutoBoundsDisabled = false;

	/** User-disabled auto-hull regeneration; surfaces a HUD warning. */
	bool bAutoHullDisabled = false;
	
	bool bAllowNonConvexHull = false;

	/** User-disabled auto-voxel regeneration; surfaces a HUD warning. */
	bool bAutoVoxelDisabled = false;

	/** Hash of the previously-selected organ set, used to detect selection changes per tick. */
	uint32 PreviousSelectedOrganHash = 0;
	
	FDelegateHandle OnLevelActorDeletedHandle;
	
	FNMultiLineTextBoxCanvasItem CanvasMessageBox = FNMultiLineTextBoxCanvasItem();
};

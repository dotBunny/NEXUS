// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorModeManager.h"
#include "NWorldAssemblyEditorColors.h"
#include "NWorldAssemblyEditorModule.h"
#include "Cell/NCellActor.h"
#include "Developer/NDebugActor.h"
#include "Tools/LegacyEdModeWidgetHelpers.h"
#include "UObject/ObjectKey.h"
#include "Visualizers/NCellRootComponentVisualizer.h"

#include "NWorldAssemblyEdMode.generated.h"

class FNAssemblyOperationContext;
class UNAssemblyOperation;
struct FPropertyChangedEvent;

/**
 * Controls how much of the World Assembly edit-mode visualization is drawn, used to produce clean captures.
 *
 * Toggled by the screenshot commands so debug overlays and on-screen messages can be suppressed for a tidy image.
 */
enum class ENWorldAssemblyEdModeRenderMode
{
	/** Full authoring view: debug overlays (bounds/hull/voxel) and HUD warning messages are drawn. */
	All,
	/** Cell-thumbnail capture: cell overlays are still drawn, but HUD messages are hidden. */
	CellScreenshot,
	/** Level capture: both debug overlays and HUD messages are hidden for a clean level screenshot. */
	LevelScreenshot,
	/** Draw nothing: neither overlays nor HUD messages are rendered. */
	None,
};

/**
 * Custom editor mode for World Assembly cell/organ authoring.
 *
 * Provides viewport overlays (bounds/hull/voxel) for the focused cell actor, orchestrates the
 * editor-side preview generation for the selected organ, and surfaces auto-generate warnings
 * to the user via on-screen HUD messages.
 *
 * Derives from UBaseLegacyWidgetEdMode — the UEdMode Epic supplies for exactly this transition — rather than from
 * the legacy FEdMode. That base implements ILegacyEdModeWidgetInterface and ILegacyEdModeViewportInterface, which is
 * what keeps Render, DrawHUD and Tick available: UEdMode itself has none of the three, and every pixel this mode
 * puts on screen goes through them.
 */
UCLASS()
class UNWorldAssemblyEdMode final : public UBaseLegacyWidgetEdMode
{
	GENERATED_BODY()

public:
	UNWorldAssemblyEdMode();

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

	/**
	 * @return The active World Assembly edit mode, or nullptr when it is not the active level-editor mode.
	 * @note This is the seam that lets the mode hold its state on the instance while the editor UI keeps calling
	 *       through static accessors. Everything below that reads or writes mode state resolves through here and
	 *       degrades to a sensible default when the mode is not up, because the visibility and CanExecute predicates
	 *       driving the toolbar are evaluated whether or not it is.
	 */
	static UNWorldAssemblyEdMode* Get();

	/** @return Cached axis-aligned bounds for the focused cell, or an empty box when the mode is not active. */
	static FBox GetCachedBounds();

	/** @return Cached bounds overlay vertex positions, or an empty array when the mode is not active. */
	static const TArray<FVector>& GetCachedBoundsVertices();

	/** @return Cached voxel overlay data, or empty data when the mode is not active. */
	static const FNCellVoxelData& GetCachedVoxelData();

	/** @return Cached hull overlay vertex positions, or an empty array when the mode is not active. */
	static const TArray<FVector>& GetCachedHullVertices();

	/** @return Cached hull overlay edge indices, or an empty array when the mode is not active. */
	static const TArray<FIntVector2>& GetCachedHullEdges();

	/** @return The cell actor the mode is currently focused on, or nullptr if none, stale, or the mode is not active. */
	static ANCellActor* GetCellActor();

	/** @return The currently active cell-edit sub-mode, or Bounds when the mode is not active. */
	static ENCellEdMode GetCellEdMode();

	/** Set the active cell-edit sub-mode. No-op when the mode is not active. */
	static void SetCellEdMode(ENCellEdMode InCellEdMode);

	/** Force the edit mode back to a safe state if the current one becomes invalid. */
	static void ProtectCellEdMode();

	/**
	 * Shut down whichever interactive tool is running. No-op when none is, or when the mode is not active.
	 * @note Completed, not Cancelled: these tools commit every edit as they make it, so there is nothing to roll back
	 *       and asking for a cancel would misreport what happened to anything listening.
	 */
	static void EndActiveTool();

	/** @return The currently active voxel visualization style, or None when the mode is not active. */
	static ENCellVoxelMode GetCellVoxelMode();

	/** Set the voxel visualization style. No-op when the mode is not active. */
	static void SetCellVoxelMode(ENCellVoxelMode InCellVoxelMode);

	/** @return true if a cell actor is currently focused. */
	static bool HasCellActor() { return GetCellActor() != nullptr; }

	/** @return true if no cell actor is currently focused. */
	static bool HasNoCellActor() { return GetCellActor() == nullptr; }

	/** @return true if this editor mode is currently the active level-editor mode. */
	static bool IsActive() { return GLevelEditorModeTools().IsModeActive(Identifier); }

	/** @return true if this editor mode is not the active level-editor mode. */
	static bool IsNotActive() { return !GLevelEditorModeTools().IsModeActive(Identifier); }

	/** @return true when a world-collision visualizer actor is currently alive for the focused world. */
	static bool HasCollisionVisualizer();

	/** Set the active render mode used to draw World Assembly debug geometry. No-op when the mode is not active. */
	static void SetRenderMode(ENWorldAssemblyEdModeRenderMode InRenderMode);

	/**
	 * @return Every warning currently applying to the focused cell, one per line, or empty text when there is nothing
	 *         to report (or no active mode).
	 * @remark Consumed by FNWorldAssemblyEdModeToolkit's warning area. This replaced the viewport HUD text the mode
	 *         used to draw over the canvas — the panel is a better home for it, and it no longer competes with the
	 *         scene or has to be suppressed for screenshots.
	 */
	static FText GetWarningText();

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
	 * Destroys the cached world-collision visualizer actor if one exists. No-op when none is alive, or when the mode is
	 * not active. Called automatically from CreateCollisionVisualizer (refresh) and during Exit() so the visualizer
	 * doesn't outlive the editor mode.
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

	//~UObject
	virtual void BeginDestroy() override;
	//End UObject

	//~UBaseLegacyWidgetEdMode
	virtual void Enter() override;
	virtual void Exit() override;

	/**
	 * @note UEdMode's own per-frame hook, rather than the ILegacyEdModeViewportInterface Tick this used to override.
	 *       Both are driven from FEditorModeTools::Tick; this one is not part of the legacy interaction surface, and
	 *       the work here never needed the FEditorViewportClient that Tick handed it.
	 */
	virtual void ModeTick(float DeltaTime) override;

	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;

	/**
	 * @return true when the key press was consumed here.
	 * @note Only handles Escape, to end the running tool. The mode's tools edit live and commit as they go, so there
	 *       is nothing to accept or discard on the way out — leaving is the whole gesture, and Escape is where every
	 *       other editor mode puts it.
	 * @remark Reached despite RequiresLegacyViewportInteractions() being false: FEditorModeTools::InputKey offers the
	 *         key to every mode implementing ILegacyEdModeViewportInterface before anything else, ungated.
	 */
	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;

	/**
	 * @return Always false: this mode draws, it does not interact.
	 * @note ILegacyEdModeViewportInterface defaults this to true, so any mode implementing it — FEdMode before, and
	 *       UBaseLegacyWidgetEdMode now — claims "I need the non-ITF gizmo and viewport input system" for free,
	 *       whether or not it is true. It is not true here: the overrides above are the whole of this mode, and none
	 *       of the legacy interaction surface (InputKey, InputDelta, Start/EndTracking, HandleClick, the transform
	 *       and property widget hooks) is implemented. Left at the default, activating the mode makes the editor
	 *       switch ITF gizmos and controls off for as long as it is open, and warn that it did.
	 * @note Porting to UEdMode did not make this override redundant. UBaseLegacyWidgetEdMode is a UEdMode, but it
	 *       still implements the legacy interface — that is what supplies Render, DrawHUD and Tick — so it inherits
	 *       the same default. Only a mode implementing no legacy interface at all avoids the claim structurally.
	 * @remark Render and DrawHUD are not part of the ITF, so the overlays are unaffected by this. The component
	 *         visualizers this mode drives are the level editor's own FComponentVisualizer system rather than the ed
	 *         mode viewport interface, and are likewise unaffected.
	 */
	virtual bool RequiresLegacyViewportInteractions() const override { return false; }

	/**
	 * @return Always true: the mode's UI lives in FNWorldAssemblyEdModeToolkit's panel.
	 * @note Turning this on is what makes the editor host the toolkit and open the Mode Toolbox panel for it — see
	 *       FModeToolkit::InvokeUI, which invokes that tab unconditionally.
	 */
	virtual bool UsesToolkits() const override { return true; }
	//End UBaseLegacyWidgetEdMode

protected:
	//~UEdMode
	virtual void CreateToolkit() override;

	/** Register the mode's interactive tools. Called from Enter(), after the tools context exists. */
	virtual void BindCommands() override;
	//End UEdMode

private:
	/**
	 * Registers a tool whose palette button toggles rather than restarts.
	 *
	 * @param UICommand Command the palette button is built from.
	 * @param ToolIdentifier Unique identifier the tool is registered under.
	 * @param Builder Builder that constructs the tool.
	 * @note UEdMode::RegisterTool maps the command straight to StartTool, so clicking an already-running tool's button
	 *       restarts it — while the button renders as lit, because its checked state is bound to IsToolActive. This
	 *       re-maps the action afterwards so the lit button ends the tool, which is what a toggle appearance promises.
	 */
	void RegisterToggleableTool(const TSharedPtr<FUICommandInfo>& UICommand, const FString& ToolIdentifier, UInteractiveToolBuilder* Builder);

private:
	/** Instance half of CreateCollisionVisualizer; see that overload for the contract. */
	TObjectPtr<ANDebugActor> RefreshCollisionVisualizer(UWorld* World);

	/** Instance half of DestroyCollisionVisualizer; see that overload for the contract. */
	void TearDownCollisionVisualizer();

	/** Subscribe to the editor world-change delegates that drive live visualizer refreshes. Called when one is spawned. */
	void BindWorldChangeDelegates();

	/** Unsubscribe from the editor world-change delegates. Called when the visualizer is destroyed or the mode exits. */
	void UnbindWorldChangeDelegates();

	/** Flag the cached visualizer for a rebuild on the next ModeTick. Bursts of changes coalesce into a single rebuild. */
	void MarkCollisionVisualizerDirty() { bCollisionVisualizerDirty = true; }

	/**
	 * @return true when a change to Actor could alter the cached visualizer — i.e. Actor currently passes the collision
	 *         filter, or it was part of the source set the live visualizer was last built from (so a delete / collision
	 *         toggle / ignore-tag still forces it to drop out). Always false while no visualizer is alive.
	 */
	bool ShouldRebuildForActor(const AActor* Actor) const;

	/** @return The actor affected by a change delegate payload — the object itself, or its owner when it is a component. */
	static AActor* ResolveAffectedActor(UObject* Object);

	/** Delegate: an actor was removed from the level — drop it from our state, or flag a refresh when it sourced one. */
	void OnActorDeleted(AActor* Actor);

	/** Delegate: a relevant actor was added to the level — flag a refresh. */
	void OnLevelActorAdded(AActor* Actor);

	/** Delegate: a transform gizmo drag ended on Object — flag a refresh when it is relevant. */
	void OnObjectMoved(UObject& Object);

	/** Delegate: a finalized (non-interactive) property edit landed on Object — flag a refresh when it is relevant. */
	void OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);

	/** Delegate: an undo/redo transaction completed — geometry can't be cheaply diffed, so always flag a refresh. */
	void OnUndoRedo();

	TArray<FVector> CachedHullVertices;
	TArray<FIntVector2> CachedHullEdges;

	FBox CachedBounds = FBox(ForceInit);
	FNCellVoxelData CachedVoxelData;
	TArray<FVector> CachedBoundsVertices;
	TWeakObjectPtr<ANCellActor> CellActor;
	ENCellEdMode CellEdMode = ENCellEdMode::Bounds;
	ENCellVoxelMode CellVoxelMode = ENCellVoxelMode::None;
	ENWorldAssemblyEdModeRenderMode RenderMode = ENWorldAssemblyEdModeRenderMode::All;

	/**
	 * The live world-collision visualizer, or nullptr when none is spawned.
	 * @note A real UPROPERTY now that this is instance state, which is what roots it. As a static TObjectPtr it could
	 *       not be one — statics are invisible to the property system — so the actor was reachable only through its
	 *       world's actor list.
	 */
	UPROPERTY()
	TObjectPtr<ANDebugActor> CollisionVisualizer;

	/** Actors that sourced the geometry of the live visualizer; lets change delegates test relevance in O(1). */
	TSet<FObjectKey> CollisionSourceActors;

	/** Set by the world-change delegates when the visualizer needs rebuilding; consumed (and cleared) in ModeTick. */
	bool bCollisionVisualizerDirty = false;

	FDelegateHandle OnLevelActorAddedHandle;
	FDelegateHandle OnObjectMovedHandle;
	FDelegateHandle OnObjectPropertyChangedHandle;
	FDelegateHandle OnUndoRedoHandle;

	/**
	 * Editor-side operation used to preview organ generation.
	 * @note A real UPROPERTY now that the mode is a UObject, which is what keeps it alive. Under FEdMode it was held
	 *       by an AddToRoot in Enter, balanced by the RemoveFromRoot inside TearDownOperation — correct, but only for
	 *       as long as every exit path reached that call.
	 */
	UPROPERTY()
	TObjectPtr<UNAssemblyOperation> OrganGenerator;

	/** Gate that lets Tick skip work when nothing relevant has changed. */
	bool bCanTick = false;

	/** True if any focused cell actor currently has unsaved dirty state. */
	bool bHasDirtyActors = false;

	/** User-disabled auto-bounds regeneration; surfaces a HUD warning. */
	bool bAutoBoundsDisabled = false;

	/** User-disabled auto-hull regeneration; surfaces a HUD warning. */
	bool bAutoHullDisabled = false;

	/** When true, the focused cell is allowed to keep a non-convex hull rather than forcing convexity. */
	bool bAllowNonConvexHull = false;

	/** User-disabled auto-voxel regeneration; surfaces a HUD warning. */
	bool bAutoVoxelDisabled = false;

	/** Hash of the previously-selected organ set, used to detect selection changes per tick. */
	uint32 PreviousSelectedOrganHash = 0;

	FDelegateHandle OnLevelActorDeletedHandle;
};

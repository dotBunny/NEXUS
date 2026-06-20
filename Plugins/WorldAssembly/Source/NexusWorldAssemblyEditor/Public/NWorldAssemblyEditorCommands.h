// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWorldAssemblyEditorStyle.h"

class UNOrganComponent;
class UNCellJunctionComponent;

/** Per-junction action delegate signature used by junction tool-menu entries. */
DECLARE_DELEGATE_OneParam( FNCellJunctionDelegate, UNCellJunctionComponent* );

/** Per-junction predicate delegate signature used to drive CanExecute/IsChecked for junction entries. */
DECLARE_DELEGATE_RetVal_OneParam(bool, FNCellJunctionBoolDelegate, UNCellJunctionComponent*);

/**
 * Slate UI commands exposed by the World Assembly editor module.
 *
 * Groups cell-actor, organ, and cell-junction commands into separate command lists so each
 * tool-menu/toolbar surface can bind only the commands it needs.
 */
class FNWorldAssemblyEditorCommands final : public TCommands<FNWorldAssemblyEditorCommands>
{
	friend class FNWorldAssemblyEditorToolMenu;

public:
	FNWorldAssemblyEditorCommands()
		: TCommands<FNWorldAssemblyEditorCommands>(
			TEXT("NWorldAssemblyEditorCommands"),
			NSLOCTEXT("NexusWorldAssemblyEditor", "NWorldAssemblyEditorCommands", "NWorldAssemblyEditorCommands"),
			NAME_None,
			FNWorldAssemblyEditorStyle::GetStyleSetName())
	{
	}

	//~TCommands
	virtual void RegisterCommands() override;
	//End TCommands

	/**
	 * Appends the Organ command bindings to the supplied (global) command list so their input chords fire
	 * regardless of which menu has focus.
	 * @param GlobalActions The level editor's global action list to append to.
	 * @remark Pair every call with UnregisterGlobalActions on the same list — the bindings reference module statics.
	 */
	void RegisterGlobalActions(const TSharedRef<FUICommandList>& GlobalActions) const;

	/** Removes the Organ command bindings previously added by RegisterGlobalActions from GlobalActions. */
	void UnregisterGlobalActions(const TSharedRef<FUICommandList>& GlobalActions) const;

	/** Toggle the World Assembly editor mode on the active level editor. */
	static void WorldAssemblyEdMode();
	/** @return true if the World Assembly-edit-mode entry should be shown in the current context. */
	static bool WorldAssemblyEdMode_CanShow();

	/** Switch the cell-editor view to the hull-edit sub-mode. */
	static void CellActorEditHullMode();
	/** @return true if the hull-edit sub-mode can be entered for the current selection. */
	static bool CellActorEditHullMode_CanExecute();

	/** Switch the cell-editor view to the bounds-edit sub-mode. */
	static void CellActorEditBoundsMode();

	/** Switch the cell-editor view to the voxel-edit sub-mode. */
	static void CellActorEditVoxelMode();

	/** Toggle whether voxel data is drawn in the viewport overlay. */
	static void CellActorToggleDrawVoxelData();

	/** Place a new ANCellActor in the focused level at a sensible default location. */
	static void CellAddActor();
	/** @return true if the add-cell-actor entry should be shown (e.g. level is editable). */
	static bool CellAddActor_CanShow();

	/** Kick off proxy generation for the currently-selected organ component(s). */
	static void OrganGenerateProxies();
	/** Kick off proxy generation for every organ component in the current level. */
	static void OrganGenerateAllProxies();
	/** Remove previously-generated proxies for the currently-selected organ component(s). */
	static void OrganClearGenerated();
	/** Remove previously-generated proxies for every organ component in the current level. */
	static void OrganClearAllProxies();
	/** Load the streaming-level proxies for the currently-selected organ component(s). */
	static void OrganLoadProxyLevels();
	/** Load the streaming-level proxies for every organ component in the current level. */
	static void OrganLoadAllProxyLevels();
	/** Unload the streaming-level proxies for the currently-selected organ component(s). */
	static void OrganUnloadProxyLevels();
	/** Unload the streaming-level proxies for every organ component in the current level. */
	static void OrganUnloadAllProxyLevels();

	/** Select the cell actor that owns the current cell-editor context. */
	static void CellSelectActor();
	/** @return true if a cell-actor selection target is available. */
	static bool CellSelectActor_CanExecute();
	/** @return true if the select-actor entry should be shown in the current context. */
	static bool CellSelectActor_CanShow();

	/** Recompute bounds, hull, and voxel data for the focused cell. */
	static void CellCalculateAll();
	/** Recompute only the focused cell's axis-aligned bounds. */
	static void CellCalculateBounds();
	/** Recompute only the focused cell's convex hull. */
	static void CellCalculateHull();

	/** Recompute only the focused cell's voxel data. */
	static void CellCalculateVoxelData();
	/** @return true if voxel recalculation is possible (e.g. bounds exist). */
	static bool CellCalculateVoxelData_CanExecute();

	/** Toggle whether the focused cell recalculates bounds automatically on world save. */
	static void CellToggleBoundsCalculateOnSave();
	/** @return checked state of the bounds-calculate-on-save toggle for UI binding. */
	static bool CellToggleBoundsCalculateOnSave_IsActionChecked();
	/** Toggle whether the focused cell recalculates the hull automatically on world save. */
	static void CellToggleHullCalculateOnSave();


	/** @return checked state of the allow-non-convex-hull toggle for UI binding. */
	static bool CellToggleHullAllowNonConvex_IsActionChecked();
	/** Toggle whether the focused cell's hull is allowed to be non-convex. */
	static void CellToggleHullAllowNonConvex();

	/** @return checked state of the hull-calculate-on-save toggle for UI binding. */
	static bool CellToggleHullCalculateOnSave_IsActionChecked();
	/** Toggle whether the focused cell recalculates voxel data automatically on world save. */
	static void CellToggleVoxelCalculateOnSave();
	/** @return checked state of the voxel-calculate-on-save toggle for UI binding. */
	static bool CellToggleVoxelCalculateOnSave_IsActionChecked();
	/** Toggle whether the focused cell stores voxel data at all (disables regeneration). */
	static void CellToggleVoxelData();
	/** @return checked state of the voxel-data-enabled toggle for UI binding. */
	static bool CellToggleVoxelData_IsActionChecked();

	/** Reset the focused cell to its default authored state (clears cached bounds/hull/voxel). */
	static void CellResetCell();

	/** Save the focused cell to its side-car package. */
	static void CellSaveCell();

	/** Remove the focused cell actor from the level, deleting its side-car package. */
	static void CellRemoveActor();

	/** Attach a new UNCellJunctionComponent to the focused cell actor. */
	static void CellJunctionAddComponent();
	/** Select the given junction component in the level editor. */
	static void CellJunctionSelectComponent(UNCellJunctionComponent* Junction);

	/** Select the given organ component in the level editor. */
	static void OrganSelectComponent(UNOrganComponent* Organ);

	/** Capture the active viewport as a thumbnail image for the focused cell's side-car. */
	static void CellCaptureThumbnail();

	/** @return true if the active viewport is suitable for thumbnail capture. */
	static bool CellCaptureThumbnail_CanExecute();
	/** Toolbar click handler: cancels the tracked Quick Assembly operation if one is running, otherwise starts a new one. */
	static void QuickAssemblyButtonClicked();
	/** @return true when the Quick Assembly button can act — always true while running (to allow cancel), else the start preconditions. */
	static bool QuickAssemblyButton_CanExecute();

	/** Start a Quick Assembly operation for the selected Organ and track its ticket for the toggle. */
	static void StartQuickAssembly();
	/** @return true if a Quick Assembly operation can be started (valid Organ, not PIE, no operation already running). */
	static bool StartQuickAssembly_CanExecute();
	/** Cancel the tracked Quick Assembly operation if it is still running. */
	static void CancelQuickAssembly();


	/** Toggle UNWorldAssemblyEditorUserSettings::bQuickAssemblyLoadLevelInstances and persist it. */
	static void QuickAssemblyToggleLoadInstances();
	/** @return checked state of the load-level-instances toggle for UI binding. */
	static bool QuickAssemblyToggleLoadInstances_IsActionChecked();
	/** Toggle UNWorldAssemblyEditorUserSettings::bQuickAssemblyAutoAssembly and persist it. */
	static void QuickAssemblyToggleAutoAssembly();
	/** @return checked state of the auto-assembly toggle for UI binding. */
	static bool QuickAssemblyToggleAutoAssembly_IsActionChecked();

private:

	/**
	 * One command paired with the delegates it binds to.
	 */
	struct FNCommandInfoAction
	{
		TSharedPtr<FUICommandInfo> CommandInfo;
		FExecuteAction Execute;
		FCanExecuteAction CanExecute;
	};

	/** @return every Organ command appended to the global action list (see FOrganGlobalAction). */
	TArray<FNCommandInfoAction> GetGlobalOrganActions() const;
	TArray<FNCommandInfoAction> GetQuickAssemblyActions() const;

	TSharedPtr<FUICommandList> CommandList_Cell;
	TSharedPtr<FUICommandInfo> CommandInfo_CellCaptureThumbnail;
	TSharedPtr<FUICommandInfo> CommandInfo_CellCalculateAll;
	TSharedPtr<FUICommandInfo> CommandInfo_CellCalculateBounds;
	TSharedPtr<FUICommandInfo> CommandInfo_CellCalculateHull;
	TSharedPtr<FUICommandInfo> CommandInfo_CellCalculateVoxelData;
	TSharedPtr<FUICommandInfo> CommandInfo_CellResetCell;
	TSharedPtr<FUICommandInfo> CommandInfo_CellSaveCell;
	TSharedPtr<FUICommandInfo> CommandInfo_CellRemoveActor;

	TSharedPtr<FUICommandInfo> CommandInfo_CellToggleBoundsCalculateOnSave;
	TSharedPtr<FUICommandInfo> CommandInfo_CellToggleHullCalculateOnSave;
	TSharedPtr<FUICommandInfo> CommandInfo_CellToggleHullAllowNonConvex;
	TSharedPtr<FUICommandInfo> CommandInfo_CellToggleVoxelCalculateOnSave;
	TSharedPtr<FUICommandInfo> CommandInfo_CellToggleVoxelData;

	TSharedPtr<FUICommandList> CommandList_QuickAssembly;
	TSharedPtr<FUICommandInfo> CommandInfo_QuickAssemblyToggleLoadInstances;
	TSharedPtr<FUICommandInfo> CommandInfo_QuickAssemblyToggleAutoAssembly;

	TSharedPtr<FUICommandList> CommandList_Organ;
	TSharedPtr<FUICommandInfo> CommandInfo_OrganGenerateProxies;
	TSharedPtr<FUICommandInfo> CommandInfo_OrganGenerateAllProxies;
	TSharedPtr<FUICommandInfo> CommandInfo_OrganClearProxies;
	TSharedPtr<FUICommandInfo> CommandInfo_OrganClearAllProxies;
	TSharedPtr<FUICommandInfo> CommandInfo_OrganCreateLevelInstances;
	TSharedPtr<FUICommandInfo> CommandInfo_OrganCreateAllLevelInstances;
	TSharedPtr<FUICommandInfo> CommandInfo_OrganUnloadLevelInstances;
	TSharedPtr<FUICommandInfo> CommandInfo_OrganUnloadAllLevelInstances;

	TSharedPtr<FUICommandList> CommandList_CellJunction;
	TSharedPtr<FUICommandInfo> CommandInfo_CellJunctionAddComponent;
	TSharedPtr<FUICommandInfo> CommandInfo_CellJunctionSelectComponent;
};

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenEditorStyle.h"

class UNOrganComponent;
class UNCellJunctionComponent;

DECLARE_DELEGATE_OneParam( FNCellJunctionDelegate, UNCellJunctionComponent* );
DECLARE_DELEGATE_RetVal_OneParam(bool, FNCellJunctionBoolDelegate, UNCellJunctionComponent*);

class FNProcGenEditorCommands final : public TCommands<FNProcGenEditorCommands>
{
	friend class FNProcGenEditorToolMenu;

public:
	FNProcGenEditorCommands()
		: TCommands<FNProcGenEditorCommands>(
			TEXT("NProcGenEditorCommands"),
			NSLOCTEXT("NexusProcGenEditor", "NProcGenEditorCommands", "NProcGenEditorCommands"),
			NAME_None,
			FNProcGenEditorStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;
	
	static void ProcGenEdMode();
	static bool ProcGenEdMode_CanShow();

	static void CellActorEditHullMode();
	static bool CellActorEditHullMode_CanExecute();	

	static void CellActorEditBoundsMode();
	
	static void CellActorEditVoxelMode();
	static void CellActorToggleDrawVoxelData();
	
	static void CellAddActor();
	static bool CellAddActor_CanShow();
	
	static void OrganGenerate();

	static void CellSelectActor();
	static bool CellSelectActor_CanExecute();
	static bool CellSelectActor_CanShow();
	
	static void CellCalculateAll();
	static void CellCalculateBounds();
	static void CellCalculateHull();
	
	static void CellCalculateVoxelData();
	static bool CellCalculateVoxelData_CanExecute();
	
	static void CellToggleBoundsCalculateOnSave();
	static bool CellToggleBoundsCalculateOnSave_IsActionChecked();
	static void CellToggleHullCalculateOnSave();
	static bool CellToggleHullCalculateOnSave_IsActionChecked();
	static void CellToggleVoxelCalculateOnSave();
	static bool CellToggleVoxelCalculateOnSave_IsActionChecked();
	static void CellToggleVoxelData();
	static bool CellToggleVoxelData_IsActionChecked();
	
	static void CellResetCell();
	static void CellRemoveActor();

	static void CellJunctionAddComponent();
	static void CellJunctionSelectComponent(UNCellJunctionComponent* Junction);
	
	static void OrganSelectComponent(UNOrganComponent* Organ);

	static void CellCaptureThumbnail();
	static bool CellCaptureThumbnail_CanExecute();

private:	
	TSharedPtr<FUICommandList> CommandList_Cell;
	TSharedPtr<FUICommandInfo> CommandInfo_CellCaptureThumbnail;
	TSharedPtr<FUICommandInfo> CommandInfo_CellCalculateAll;
	TSharedPtr<FUICommandInfo> CommandInfo_CellCalculateBounds;
	TSharedPtr<FUICommandInfo> CommandInfo_CellCalculateHull;
	TSharedPtr<FUICommandInfo> CommandInfo_CellCalculateVoxelData;
	TSharedPtr<FUICommandInfo> CommandInfo_CellResetCell;
	TSharedPtr<FUICommandInfo> CommandInfo_CellRemoveActor;
	
	
	TSharedPtr<FUICommandInfo> CommandInfo_CellToggleBoundsCalculateOnSave;
	TSharedPtr<FUICommandInfo> CommandInfo_CellToggleHullCalculateOnSave;
	TSharedPtr<FUICommandInfo> CommandInfo_CellToggleVoxelCalculateOnSave;
	TSharedPtr<FUICommandInfo> CommandInfo_CellToggleVoxelData;
	
	TSharedPtr<FUICommandList> CommandList_Organ;
	TSharedPtr<FUICommandInfo> CommandInfo_OrganGenerate;
	
	TSharedPtr<FUICommandList> CommandList_CellJunction;
	TSharedPtr<FUICommandInfo> CommandInfo_CellJunctionAddComponent;
	TSharedPtr<FUICommandInfo> CommandInfo_CellJunctionSelectComponent;
};

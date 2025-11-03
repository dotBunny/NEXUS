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
public:
	FNProcGenEditorCommands()
		: TCommands<FNProcGenEditorCommands>(
			TEXT("NProcGenEditorCommands"),
			NSLOCTEXT("Contexts", "NProcGenEditorCommands", "NProcGenEditorCommands"),
			NAME_None,
			FNProcGenEditorStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

	static void ProcGenEdMode();
	static bool ProcGenEdMode_CanExecute();
	static bool ProcGenEdMode_CanShow();

	static void CellActorEditHullMode();	
	static FSlateIcon CellActorEditHullMode_GetIcon();
	static void CellActorEditBoundsMode();
	static FSlateIcon CellActorEditBoundsMode_GetIcon();
	
	static void CellActorToggleDrawVoxelData();
	static FSlateIcon CellActorToggleDrawVoxelData_GetIcon();
	
	static void CellAddActor();
	static bool CellAddActor_CanExecute();
	static bool CellAddActor_CanShow();
	
	static void OrganGenerate();
	static bool OrganGenerate_CanExecute();

	static void CellSelectActor();
	static bool CellSelectActor_CanExecute();
	static bool CellSelectActor_CanShow();
	
	static void CellCalculateAll();
	static bool CellCalculateAll_CanExecute();
	
	static void CellCalculateBounds();
	static bool CellCalculateBounds_CanExecute();
	
	static void CellCalculateHull();
	static bool CellCalculateHull_CanExecute();
	
	static void CellCalculateVoxelData();
	static bool CellCalculateVoxelData_CanExecute();
	
	static void CellResetCell();
	static bool CellResetCell_CanExecute();
	
	static void CellRemoveActor();
	static bool CellRemoveActor_CanExecute();

	static void CellJunctionAddComponent();
	static bool CellJunctionAddComponent_CanExecute();

	static void CellJunctionSelectComponent(UNCellJunctionComponent* Junction);
	static bool CellJunctionSelectComponent_CanExecute(UNCellJunctionComponent* Junction);
	
	static void OrganSelectComponent(UNOrganComponent* Organ);
	static bool OrganSelectComponent_CanExecute(UNOrganComponent* Organ);

	static void CellCaptureThumbnail();
	static bool CellCaptureThumbnail_CanExecute();
	
	TSharedPtr<FUICommandList> CommandList_Cell;
	TSharedPtr<FUICommandInfo> CommandInfo_CellCaptureThumbnail;
	TSharedPtr<FUICommandInfo> CommandInfo_CellCalculateAll;
	TSharedPtr<FUICommandInfo> CommandInfo_CellCalculateBounds;
	TSharedPtr<FUICommandInfo> CommandInfo_CellCalculateHull;
	TSharedPtr<FUICommandInfo> CommandInfo_CellCalculateVoxelData;
	TSharedPtr<FUICommandInfo> CommandInfo_CellResetCell;
	TSharedPtr<FUICommandInfo> CommandInfo_CellRemoveActor;
	
	TSharedPtr<FUICommandList> CommandList_Organ;
	TSharedPtr<FUICommandInfo> CommandInfo_OrganGenerate;
	
	TSharedPtr<FUICommandList> CommandList_CellJunction;
	TSharedPtr<FUICommandInfo> CommandInfo_CellJunctionAddComponent;
	TSharedPtr<FUICommandInfo> CommandInfo_CellJunctionSelectComponent;
};

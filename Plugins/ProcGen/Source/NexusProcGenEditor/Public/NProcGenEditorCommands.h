// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenEditorStyle.h"

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

	static void NProcGenEdMode();
	static bool NProcGenEdMode_CanExecute();
	static bool NProcGenEdMode_CanShow();

	static void NCellActorEditHullMode();	
	static FSlateIcon NCellActorEditHullMode_GetIcon();
	static void NCellActorEditBoundsMode();
	static FSlateIcon NCellActorEditBoundsMode_GetIcon();

	static void NCellAddActor();
	static bool NCellAddActor_CanExecute();
	static bool NCellAddActor_CanShow();

	static void NCellSelectActor();
	static bool NCellSelectActor_CanExecute();
	static bool NCellSelectActor_CanShow();
	
	
	static void OnNCellCalculateAll();
	static bool OnNCellCalculateAll_CanExecute();
	
	static void OnNCellCalculateBounds();
	static bool OnNCellCalculateBounds_CanExecute();
	
	static void OnNCellCalculateHull();
	static bool OnNCellCalculateHull_CanExecute();
	
	static void OnNCellResetCell();
	static bool OnNCellResetCell_CanExecute();
	
	static void OnNCellRemoveActor();
	static bool OnNCellRemoveActor_CanExecute();

	static void OnNCellJunctionAddComponent();
	static bool OnNCellJunctionAddComponent_CanExecute();

	static void OnNCellJunctionSelectComponent(UNCellJunctionComponent* Junction);
	static bool OnNCellJunctionSelectComponent_CanExecute(UNCellJunctionComponent* Junction);
	
	TSharedPtr<FUICommandList> CommandList_NCell;
	
	TSharedPtr<FUICommandInfo> CommandInfo_NCellCalculateAll;
	TSharedPtr<FUICommandInfo> CommandInfo_NCellCalculateBounds;
	TSharedPtr<FUICommandInfo> CommandInfo_NCellCalculateHull;
	TSharedPtr<FUICommandInfo> CommandInfo_NCellResetCell;
	TSharedPtr<FUICommandInfo> CommandInfo_NCellRemoveActor;


	TSharedPtr<FUICommandList> CommandList_NCellJunction;
	TSharedPtr<FUICommandInfo> CommandInfo_NCellJunctionAddComponent;
	TSharedPtr<FUICommandInfo> CommandInfo_NCellJunctionSelectComponent;
};

﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NFixersEditorStyle.h"

class FNFixersEditorCommands final : public TCommands<FNFixersEditorCommands>
{
public:
	FNFixersEditorCommands()
		: TCommands<FNFixersEditorCommands>(
			TEXT("NFixersEditorCommands"),
			NSLOCTEXT("Contexts", "NFixersEditorCommands", "NFixersEditorCommands"),
			NAME_None,
			FNFixersEditorStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;
	
	TSharedPtr<FUICommandList> CommandList_BulkOperations;
	
	TSharedPtr<FUICommandInfo> CommandInfo_BulkOperations_PoseAsset_OutOfDateAnimationSource;
	TSharedPtr<FUICommandInfo> CommandInfo_BulkOperations_PoseAsset_OutOfDateAnimationSource_NoContext;
};

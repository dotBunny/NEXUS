// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Cell/NCellActor.h"

class UNCell;

class NEXUSPROCGENEDITOR_API FNProcGenEditorUtils
{
public:
	static bool IsNCellActorPresentInCurrentWorld();
	static ANCellActor* GetNCellActorFromCurrentWorld();
	static bool IsNCellActorSelected();
	
	static void SaveNCell(UWorld* World, ANCellActor* CellActor = nullptr);
	static bool UpdateNCell(UNCell* Cell, ANCellActor* CellActor);
};

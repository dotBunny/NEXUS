// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNProcGenEditorToolMenu
{
public:
	static void AddMenuEntries();
	static void RemoveMenuEntries();
	
	static bool ShowCellEditMode();
	static bool ShowDrawVoxels();
	static bool ShowCellDropdown();
	static bool ShowCellJunctionDropdown();
	static bool ShowOrganDropdown();
	
	
	static void CreateEditorUtilityWindow();
	static bool HasEditorUtilityWindow();
};

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNProcGenEditorToolMenu
{
	static FName EditorUtilityWindowName;
public:
	static void Register();
	static bool ShowCellEditMode();
	static bool ShowDrawVoxels();
	static bool ShowCellDropdown();
	static bool ShowCellJunctionDropdown();
	static bool ShowOrganDropdown();
	
	
	static void CreateEditorUtilityWindow();
	static bool HasEditorUtilityWindow();
};

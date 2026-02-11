// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNUIEditorCommands
{
public:
	static void AddMenuEntries();
	static void RemoveMenuEntries();
	
	static void CreateCollisionQueryWindow();
	static bool HasCollisionQueryWindow();
};
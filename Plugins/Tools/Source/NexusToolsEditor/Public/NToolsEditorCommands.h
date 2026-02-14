// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNToolsEditorCommands
{
public:
	static void AddMenuEntries();
	static void RemoveMenuEntries();
	
	static void CreateCollisionVisualizerWindow();
	static bool HasCollisionVisualizerWindow();
	
	static void OpenNetworkProfiler();
	static bool HasToolsProfileNetworkProfiler();
	
	static void GenerateProjectLevelsSubMenu(UToolMenu* Menu);
	
	static void OnNodeExternalDocumentation();
	static bool NodeExternalDocumentation_CanExecute();

};
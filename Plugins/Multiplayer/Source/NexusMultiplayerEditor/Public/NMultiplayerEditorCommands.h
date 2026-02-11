// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNMultiplayerEditorCommands
{
public:

	static void Register();
	static bool IsMultiplayerTestRunning() { return bIsMultiplayerTestRunning; }
	
private:
	static bool bIsMultiplayerTestRunning;
	static FText MultiplayerTest_GetTooltip();
	static FSlateIcon MultiplayerTest_GetIcon();
	static void ToggleMultiplayerTest();
};
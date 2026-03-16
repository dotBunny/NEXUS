// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNMultiplayerEditorCommands
{
public:

	static void Register();
	static void Unregister();
	
private:
	static bool MultiplayerTest_CanExecute();
	static FText MultiplayerTest_GetTooltip();
	static FSlateIcon MultiplayerTest_GetIcon();
	static void ToggleMultiplayerTest();
	
	static void OnMultiplayerTestStarted();
	static void OnMultiplayerTestEnded();
	
	static bool bIsTestRunning;
	
	static FDelegateHandle OnTestStartedHandle;
	static FDelegateHandle OnTestEndedHandle;
};
﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNMultiplayerEditorToolMenu
{
public:

	static void Register();
	static bool IsMultiplayerTestRunning() { return bIsMultiplayerTestRunning; }
	
private:
	static bool bIsMultiplayerTestRunning;
	static bool MultiplayerTest_CanExecute();
	static FText MultiplayerTest_GetTooltip();
	static FSlateIcon MultiplayerTest_GetIcon();
	static void ToggleMultiplayerTest();
};
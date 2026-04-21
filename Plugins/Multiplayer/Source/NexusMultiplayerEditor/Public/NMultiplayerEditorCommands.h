// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Editor commands for the NexusMultiplayer plugin.
 *
 * Wires the multiplayer test toggle into the editor's tool menu and bridges between the
 * menu command and the UNMultiplayerEditorSubsystem that actually starts/stops the test.
 */
class FNMultiplayerEditorCommands
{
public:
	/** Register the multiplayer test entries with UToolMenus. */
	static void Register();
	/** Unregister entries previously installed by Register. */
	static void Unregister();

private:
	/** @return true if the multiplayer test entry is currently enabled (e.g. not already running an incompatible mode). */
	static bool MultiplayerTest_CanExecute();
	/** @return Localized tooltip for the multiplayer test entry, reflecting current running state. */
	static FText MultiplayerTest_GetTooltip();
	/** @return Slate icon for the multiplayer test entry, reflecting current running state. */
	static FSlateIcon MultiplayerTest_GetIcon();
	/** Toggle the multiplayer test session on or off via the editor subsystem. */
	static void ToggleMultiplayerTest();

	/** Callback invoked when the multiplayer test session starts; updates cached state and refreshes UI. */
	static void OnMultiplayerTestStarted();
	/** Callback invoked when the multiplayer test session ends; updates cached state and refreshes UI. */
	static void OnMultiplayerTestEnded();

	/** Cached running state used to drive icon/tooltip without querying the subsystem each frame. */
	static bool bIsTestRunning;

	/** Handle for the started delegate binding, held so it can be removed on Unregister. */
	static FDelegateHandle OnTestStartedHandle;
	/** Handle for the ended delegate binding, held so it can be removed on Unregister. */
	static FDelegateHandle OnTestEndedHandle;
};
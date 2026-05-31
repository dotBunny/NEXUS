// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Bootstrap point for the default Fixers-menu entries shipped with NexusTooling. Called from the
 * editor module during startup; individual entries are registered via FNFixersMenu::AddMenuEntry.
 */
class FNFixersMenuEntries
{
public:
	/** Register the built-in Fixers entries with FNFixersMenu. */
	static void AddMenuEntries();
};
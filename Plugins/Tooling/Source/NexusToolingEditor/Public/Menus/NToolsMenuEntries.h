// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * A default set of NEXUS "Tools" dropdown menu entries, called from the NexusToolingEditor module startup.
 */
class FNToolsMenuEntries
{
public:
	/** Register the built-in Tools entries with FNToolsMenu. */
	static void AddMenuEntries();

	/** Delete the contents of Saved/Logs; bound to the "Clean Logs" menu entry. */
	static void OnWindowCleanLogsFolder();
};
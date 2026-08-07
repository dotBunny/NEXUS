// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class UNOrganComponent;

/**
 * The author-time operations an organ command performs on the level's organs and their generated output.
 *
 * Each generate/clear/load/unload action comes in a pair: one driven by the current selection, one covering every
 * organ in the world. Both route through UNWorldAssemblyEditorSubsystem, which owns the operation lifetime.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEditorOrganOperations
{
public:
	/** Place a new ANOrganVolume in front of the viewport and select it. */
	static void AddVolume();

	/** Kick off proxy generation for the currently-selected organ component(s). */
	static void GenerateProxies();
	/** Kick off proxy generation for every organ component in the current level. */
	static void GenerateAllProxies();

	/** Remove previously-generated proxies for the currently-selected organ component(s). */
	static void ClearGenerated();
	/** Remove previously-generated proxies for every organ component in the current level. */
	static void ClearAllProxies();

	/** Load the streaming-level proxies for the currently-selected organ component(s). */
	static void LoadProxyLevels();
	/** Load the streaming-level proxies for every organ component in the current level. */
	static void LoadAllProxyLevels();

	/** Unload the streaming-level proxies for the currently-selected organ component(s). */
	static void UnloadProxyLevels();
	/** Unload the streaming-level proxies for every organ component in the current level. */
	static void UnloadAllProxyLevels();

	/**
	 * Select the given organ component in the level editor.
	 * @param Organ The organ to make the sole selection.
	 */
	static void SelectComponent(UNOrganComponent* Organ);
};

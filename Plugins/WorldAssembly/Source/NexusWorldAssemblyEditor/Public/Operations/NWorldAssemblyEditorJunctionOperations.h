// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class UNCellJunctionComponent;

/**
 * The author-time operations a junction command performs on the focused cell's junctions.
 *
 * Companion to FNWorldAssemblyEditorCellOperations for the junction category: the commands in
 * FNWorldAssemblyEditorJunctionCommands declare, bind and gate; these do the work.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEditorJunctionOperations
{
public:
	/** Attach a new UNCellJunctionComponent to every selected actor, then select the components that were added. */
	static void AddComponent();

	/**
	 * Select the given junction component in the level editor.
	 * @param Junction The junction to make the sole selection.
	 */
	static void SelectComponent(UNCellJunctionComponent* Junction);

	/**
	 * Move every junction in the level onto the first selected actor, preserving world transforms.
	 * @note Prompts for confirmation, and refuses a location-locked target outright — a junction parented to one
	 *       cannot be moved in the editor afterwards.
	 */
	static void CollectComponents();
};

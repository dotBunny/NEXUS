// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Identifiers used when launching/tracking the engine's network profiler tab.
 */
namespace NEXUS::ToolingEditor::NetworkProfiler
{
	/** Stable identifier for the network-profiler tab spawned by FNToolingEditorCommands. */
	inline FName Identifier = TEXT("Tools.Profile.NetworkProfiler");
}

/**
 * Asset paths and identifiers for the collision-visualizer editor-utility widget.
 */
namespace NEXUS::ToolingEditor::CollisionVisualizer
{
	/** Content-browser path of the EUW that hosts the collision visualizer UI. */
	inline FString Path = TEXT("/NexusTooling/EditorResources/EUW_NCollisionVisualizer.EUW_NCollisionVisualizer");

	/** Stable tab identifier used when spawning/focusing the collision visualizer window. */
	inline FName Identifier = TEXT("EUW_NCollisionVisualizer");

	/** Slate brush name used for the collision visualizer tab icon. */
	inline FName Icon = TEXT("ClassIcon.NCollisionVisualizerActor");
}

/** Log category for the NexusToolingEditor module. */
NEXUSTOOLINGEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusToolingEditor, Log, All);
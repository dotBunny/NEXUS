// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Constants shared across the NexusWorldAssembly runtime module.
 */
namespace NEXUS::WorldAssembly
{
	namespace Tags
	{
		inline const FName CellIgnoreActorTag = FName("NCell_Ignore");
	}
	/**
	 * Debug-draw tuning.
	 */
	namespace Debug
	{
		/** Line thickness (cm) used when drawing World Assembly debug visualization. */
		constexpr float LineThickness = 1.5f;
	}

	/**
	 * Stable display strings reported by the developer overlay and status UI during a generation pass.
	 */
	namespace DisplayMessages
	{
		/** Emitted while the generation context is frozen and being queued up. */
		inline const FString ContextLocked = TEXT("Context Locked");
		/** Emitted while the task graph for the current pass is being assembled. */
		inline const FString BuildingTaskGraph = TEXT("Building Task Graph");
		/** Emitted immediately before scheduled tasks begin executing. */
		inline const FString StartingTasks = TEXT("Starting Tasks");
	}

	/**
	 * Stable state identifiers used when tracking operation lifecycle in logs and UI.
	 */
	namespace States
	{
		inline const FString None = TEXT("None");
		inline const FString Registered = TEXT("Registered");
		inline const FString Started = TEXT("Started");
		inline const FString Updated = TEXT("Updated");
		inline const FString Finished = TEXT("Finished");
		inline const FString Unregistered = TEXT("Unregistered");
	}

	/**
	 * Named World Assembly operations registered with the subsystem.
	 */
	namespace Operations
	{
		/** The main runtime generation operation. */
		inline const FName Main = FName("NWorldAssemblyMainOperation");
		/** The editor-mode preview generation operation. */
		inline const FName EditorMode = FName("NWorldAssemblyEditorModeOperation");
	}
}

/** Log category used by all NexusWorldAssembly logging. */
NEXUSWORLDASSEMBLY_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusWorldAssembly, Log, All);
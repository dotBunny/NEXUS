// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Constants shared across the NexusProcGen runtime module.
 */
namespace NEXUS::ProcGen
{
	/**
	 * Debug-draw tuning.
	 */
	namespace Debug
	{
		/** Line thickness (cm) used when drawing ProcGen debug visualization. */
		constexpr float LineThickness = 1.5f;
	}

	/**
	 * Stable display strings reported by the developer overlay and status UI during a generation pass.
	 */
	namespace DisplayMessages
	{
		/** Emitted while the generation context is frozen and being queued up. */
		const FString ContextLocked = TEXT("Context Locked");
		/** Emitted while the task graph for the current pass is being assembled. */
		const FString BuildingTaskGraph = TEXT("Building Task Graph");
		/** Emitted immediately before scheduled tasks begin executing. */
		const FString StartingTasks = TEXT("Starting Tasks");
	}

	/**
	 * Stable state identifiers used when tracking operation lifecycle in logs and UI.
	 */
	namespace States
	{
		const FString None = TEXT("None");
		const FString Registered = TEXT("Registered");
		const FString Started = TEXT("Started");
		const FString Updated = TEXT("Updated");
		const FString Finished = TEXT("Finished");
		const FString Unregistered = TEXT("Unregistered");
	}

	/**
	 * Named ProcGen operations registered with the subsystem.
	 */
	namespace Operations
	{
		/** The main runtime generation operation. */
		const FName Main = FName("NProcGenMainOperation");
		/** The editor-mode preview generation operation. */
		const FName EditorMode = FName("NProcGenEditorModeOperation");
	}
}

/** Log category used by all NexusProcGen logging. */
NEXUSPROCGEN_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusProcGen, Log, All);
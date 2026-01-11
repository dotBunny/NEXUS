// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

namespace NEXUS::ProcGen
{
	namespace Debug
	{
		constexpr float LineThickness = 1.5f;
	}
	
	namespace DisplayMessages
	{
		const FString ContextLocked = TEXT("Context Locked");
		const FString BuildingTaskGraph = TEXT("Building Task Graph");
		const FString StartingTasks = TEXT("Starting Tasks");
	}
	
	namespace States
	{
		const FString None = TEXT("None");
		const FString Registered = TEXT("Registered");
		const FString Started = TEXT("Started");
		const FString Updated = TEXT("Updated");
		const FString Finished = TEXT("Finished");
		const FString Unregistered = TEXT("Unregistered");
	}
	
	namespace Operations
	{
		const FName Main = FName("NProcGenMainOperation");
		const FName EditorMode = FName("NProcGenEditorModeOperation");
	}
}

NEXUSPROCGEN_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusProcGen, Log, All);
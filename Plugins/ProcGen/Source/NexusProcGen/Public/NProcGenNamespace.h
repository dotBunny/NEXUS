// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

namespace NEXUS::ProcGen::DisplayMessages
{
	const FString ContextLocked = TEXT("Context Locked");
	const FString BuildingTaskGraph = TEXT("Building Task Graph");
	const FString StartingTasks = TEXT("Starting Tasks");
}

namespace NEXUS::ProcGen::States
{
	const FString None = TEXT("None");
	const FString Registered = TEXT("Registered");
	const FString Started = TEXT("Started");
	const FString Updated = TEXT("Updated");
	const FString Finished = TEXT("Finished");
	const FString Unregistered = TEXT("Unregistered");
}

namespace NEXUS::ProcGen::Operations
{
	const FName EditorMode = FName("NProcGenEditorModeOperation");
}
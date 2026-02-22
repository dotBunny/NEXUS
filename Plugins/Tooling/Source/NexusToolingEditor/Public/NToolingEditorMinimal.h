// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once


namespace NEXUS::ToolingEditor::NetworkProfiler
{
	inline FName Identifier = TEXT("Tools.Profile.NetworkProfiler");
}

namespace NEXUS::ToolingEditor::CollisionVisualizer
{
	inline FString Path = TEXT("/NexusTooling/EditorResources/EUW_NCollisionVisualizer.EUW_NCollisionVisualizer");
	inline FName Identifier = TEXT("EUW_NCollisionVisualizer");
	inline FName Icon = TEXT("ClassIcon.NCollisionVisualizerActor");
}

NEXUSTOOLINGEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusToolingEditor, Log, All);
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once


namespace NEXUS::ToolsEditor::NetworkProfiler
{
	inline FName Identifier = TEXT("Tools.Profile.NetworkProfiler");
}

namespace NEXUS::ToolsEditor::CollisionVisualizer
{
	inline FString Path = TEXT("/NexusTools/EditorResources/EUW_NCollisionVisualizer.EUW_NCollisionVisualizer");
	inline FName Identifier = TEXT("EUW_NCollisionVisualizer");
	inline FName Icon = TEXT("ClassIcon.NCollisionVisualizerActor");
}

NEXUSTOOLSEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusToolsEditor, Log, All);
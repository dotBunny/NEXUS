// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Constants identifying the DynamicRefs editor utility widget used by editor menus/commands.
 */
namespace NEXUS::DynamicRefsEditor::EUW
{
	/** Asset path of the DynamicRefs editor utility widget. */
	inline FString Path = TEXT("/NexusDynamicRefs/EditorResources/EUW_NDynamicRefs.EUW_NDynamicRefs");
	/** Stable identifier used when registering the widget's tab. */
	inline FName Identifier = TEXT("EUW_NDynamicRefs");
	/** Slate icon used for the widget's menu/tab entry. */
	inline FName Icon = TEXT("ClassIcon.NDynamicRefComponent");
}
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Constants identifying the Guardian editor utility widget used by editor menus/commands.
 */
namespace NEXUS::Guardian::EUW
{
	/** Asset path of the Guardian editor utility widget. */
	inline FString Path = TEXT("/NexusGuardian/EditorResources/EUW_NGuardian.EUW_NGuardian");
	/** Stable identifier used when registering the widget's tab. */
	inline FName Identifier = TEXT("EUW_NGuardian");
	/** Slate icon used for the widget's menu/tab entry. */
	inline FName Icon = TEXT("Icon.NGuardian");
}

/** Log category used by all NexusGuardianEditor module. */
DECLARE_LOG_CATEGORY_EXTERN(LogNexusGuardianEditor, Log, All);
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

/**
 * Constants identifying the Actor Pools editor utility widget used by the editor menu/commands.
 */
namespace NEXUS::ActorPoolsEditor::EditorUtilityWidget
{
	/** Asset path of the Actor Pools editor utility widget. */
	inline FString Path = TEXT("/NexusActorPools/EditorResources/EUW_NActorPools.EUW_NActorPools");
	/** Stable identifier used when registering the widget's tab. */
	inline FName Identifier = TEXT("EUW_NActorPools");
	/** Slate icon used for the widget's menu/tab entry. */
	inline FName Icon = TEXT("ClassIcon.NActorPool");
}

/** Log category used by all NexusActorPoolsEditor module. */
NEXUSACTORPOOLSEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusActorPoolsEditor, Log, All);
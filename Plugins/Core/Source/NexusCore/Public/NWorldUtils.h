// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"


/**
 * A collection of native utility methods for working with worlds.
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/world-utils/">FNWorldUtils</a>
 */
class FNWorldUtils
{
public:

	/** @return The first active Game or PIE world, or nullptr if none exists. */
	static UWorld* GetGameWorld()
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
			{
				return Context.World();
			}
		}
		return nullptr;
	}

	/**
	 * Reports whether any of the world's streaming levels is still resolving — either loaded but not yet
	 * visible while it wants to be, or with a load request still pending.
	 * @param InWorld The world to inspect; a null world reports false.
	 * @return true while streaming work is in flight.
	 */
	static bool IsStreaming(const UWorld* InWorld)
	{
		if (!InWorld) return false;

		const TArray<ULevelStreaming*>& StreamingLevels = InWorld->GetStreamingLevels();
		for (ULevelStreaming* Level : StreamingLevels)
		{
			if (Level)
			{
				if (Level->IsLevelLoaded() && !Level->IsLevelVisible() && Level->ShouldBeVisible())
				{
					return true;
				}

				if (Level->HasLoadRequestPending())
				{
					return true;
				}
			}
		}
		return false;
	}
};
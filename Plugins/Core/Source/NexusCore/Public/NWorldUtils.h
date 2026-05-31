// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"


/**
 * A collection of native utility methods for working with worlds.
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
};
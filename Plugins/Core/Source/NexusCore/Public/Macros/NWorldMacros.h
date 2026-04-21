// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Resolves the UWorld a WorldContextObject belongs to with build-config appropriate behaviour.
 *
 * In shipping builds this is the raw WorldContextObject->GetWorld() call, which is faster but
 * assumes the context is valid. Non-shipping builds use GEngine->GetWorldFromContextObject with
 * EGetWorldErrorMode::LogAndReturnNull so that invalid contexts are logged during development.
 *
 * @param WorldContextObject The UObject supplying the world-context lookup.
 */
#if UE_BUILD_SHIPPING
	#define N_GET_WORLD_FROM_CONTEXT(WorldContextObject) \
		WorldContextObject->GetWorld()
#else // !UE_BUILD_SHIPPING
	#define N_GET_WORLD_FROM_CONTEXT(WorldContextObject) \
		GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)
#endif // UE_BUILD_SHIPPING
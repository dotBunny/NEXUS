// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if UE_BUILD_SHIPPING
	#define N_GET_WORLD_FROM_CONTEXT(WorldContextObject) \
		WorldContextObject->GetWorld()
#else // UE_BUILD_SHIPPING
	#define N_GET_WORLD_FROM_CONTEXT(WorldContextObject) \
		GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)	
#endif // UE_BUILD_SHIPPING
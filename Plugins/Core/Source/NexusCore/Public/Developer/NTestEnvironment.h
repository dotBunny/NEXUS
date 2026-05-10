// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

struct NEXUSCORE_API FNTestEnvironment
{
	UWorld* World = nullptr;
	FWorldContext* WorldContext = nullptr;
	UGameInstance* GameInstance = nullptr;
	
	bool bHasInitializedStackWalking = false;
	
	void InitializeStackWalking()
	{
		if (!bHasInitializedStackWalking)
		{
			bHasInitializedStackWalking = FPlatformStackWalk::InitStackWalking();
		}
	}
};
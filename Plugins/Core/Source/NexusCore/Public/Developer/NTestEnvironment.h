// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_TESTS

/**
 * Scratch world state shared across a latent automation test.
 *
 * Holds the UWorld a test is running against along with its context and game instance, plus a flag
 * recording whether stack walking has been warmed up. Populated by the create-world latent command
 * and torn down by FNTestLatentCommand_CleanupWorld -- tests should not free these themselves.
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/developer/test-environment/">FNTestEnvironment</a>
 */
struct NEXUSCORE_API FNTestEnvironment
{
	// Cleaned up in FNTestLatentCommand_CleanupWorld
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

#endif // WITH_TESTS
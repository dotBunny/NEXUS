// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NMersenneTwisterFriendlyState.generated.h"

struct FNMersenneTwisterState;

/**
 * A Blueprint-friendly, human-presentable form of FNMersenneTwisterState.
 *
 * Stores the same information (originating seed and engine draw count) but as an editor-visible,
 * hexadecimal seed string and a plain draw count, so it can ride UE serialization (SaveGame, assets,
 * JSON) and be shown to or edited by a user. Convert to/from the native FNMersenneTwisterState at the
 * native boundary via ToNative / FromNative.
 * @see FNMersenneTwisterState
 */
USTRUCT(BlueprintType, DisplayName = "NEXUS | Mersenne Twister Friendly State")
struct NEXUSCORE_API FNMersenneTwisterFriendlyState
{
	GENERATED_BODY()

	/** The originating seed, as a presentable hexadecimal string (matches FNMersenneTwister::GetSeedAsString). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Core")
	FString InitialSeed;

	/** The number of engine draws taken since the seed was set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Core")
	int64 DrawCount = 0;

	/**
	 * Converts this friendly form to the native snapshot consumed by FNMersenneTwister::RestoreState.
	 * @return the equivalent native FNMersenneTwisterState.
	 */
	FNMersenneTwisterState ToNative() const;

	/**
	 * Builds a friendly form from a native snapshot produced by FNMersenneTwister::SaveState.
	 * @param State The native snapshot to convert.
	 * @return the equivalent presentable FNMersenneTwisterFriendlyState.
	 */
	static FNMersenneTwisterFriendlyState FromNative(const FNMersenneTwisterState& State);
};

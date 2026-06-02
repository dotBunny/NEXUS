// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * The arithmetic operation an FNGameplayTagCounterOperation applies to a tag's count.
 */
UENUM(BlueprintType)
enum class ENGameplayTagCounterOperationType : uint8
{
	/** Add the operation's value to the tag's current count. */
	Add = 0,
	/** Subtract the operation's value from the tag's current count. */
	Subtract = 1,
};
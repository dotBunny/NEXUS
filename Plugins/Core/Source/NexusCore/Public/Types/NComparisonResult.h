// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Relational operator selector used to test a value against a configured threshold.
 *
 * Names which comparison to apply between a queried value and an operand; consumed by predicate checks such as
 * FNGameplayTagCounterConstraint::DoesPassComparison. It selects the operation to perform, not the outcome of one.
 */
UENUM(BlueprintType)
enum class ENComparisonResult : uint8
{
	/** Passes when the value equals the operand. */
	Equal = 0,
	/** Passes when the value is strictly greater than the operand. */
	GreaterThan = 1,
	/** Passes when the value is greater than or equal to the operand. */
	GreaterThanOrEqual = 2	UMETA(DisplayName = "Greater Than Or Equal"),
	/** Passes when the value is strictly less than the operand. */
	LessThan= 3,
	/** Passes when the value is less than or equal to the operand. */
	LessThanOrEqual = 4		UMETA(DisplayName = "Less Than Or Equal"),
	/** Passes when the value differs from the operand. */
	NotEqual = 5
};
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRotationConstraints.generated.h"



/**
 * Dual-interval rotation constraints used by ProcGen matching rules.
 *
 * The "matching" interval constrains a candidate rotation's own pose, while the "difference"
 * interval constrains the delta between two rotations. Either interval can be enabled independently.
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNRotationConstraints
{
	GENERATED_BODY()

	/** Enables the "matching" interval test on the candidate rotation itself. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Matching", DisplayName = "Enforce Matching?")
	bool bEnforceMatchingRotation = true;

	/** Lower bound of the matching interval (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Matching", DisplayName = "Minimum Matching Rotation", meta=(EditCondition="bEnforceMatchingRotation", EditConditionHides))
	FRotator MinimumMatchingRotation = FRotator(0.f, -180.f, 0.f);

	/** Upper bound of the matching interval (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Matching", DisplayName = "Maximum Matching Rotation", meta=(EditCondition="bEnforceMatchingRotation", EditConditionHides))
	FRotator MaximumMatchingRotation = FRotator(0.f, 180.f, 0.f);

	/** Enables the "difference" interval test on the delta between two rotations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difference", DisplayName = "Enforce Difference?")
	bool bEnforceDifferenceRotation = false;

	/** Lower bound of the difference interval (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difference", DisplayName = "Minimum Difference Rotation", meta=(EditCondition="bEnforceDifferenceRotation", EditConditionHides))
	FRotator MinimumDifferenceRotation  = FRotator(0.f, -180.f, 0.f);

	/** Upper bound of the difference interval (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difference", DisplayName = "Maximum Difference Rotation", meta=(EditCondition="bEnforceDifferenceRotation", EditConditionHides))
	FRotator MaximumDifferenceRotation = FRotator(0.f, 180.f, 0.f);

	/**
	 * Copies this constraint set to Other.
	 * @param Other Destination instance.
	 * @return Always true.
	 */
	bool CopyTo(FNRotationConstraints& Other) const
	{
		Other = *this;
		return true;
	}

	/**
	 * Full-field equality comparison.
	 * @param Other Right-hand side of the comparison.
	 */
	bool IsEqual(const FNRotationConstraints& Other) const
	{
		return  bEnforceMatchingRotation == Other.bEnforceMatchingRotation &&
				bEnforceDifferenceRotation == Other.bEnforceDifferenceRotation &&
				MinimumMatchingRotation == Other.MinimumMatchingRotation &&
				MaximumMatchingRotation == Other.MaximumMatchingRotation &&
				MinimumDifferenceRotation == Other.MinimumDifferenceRotation &&
				MaximumDifferenceRotation == Other.MaximumDifferenceRotation;
	}

	/**
	 * Tests whether Rotation lies within the matching interval.
	 * @param Rotation Rotation to check (axes will be normalized internally).
	 * @return true when the matching test is disabled or the rotation fits the interval.
	 */
	bool IsMatchingRotationAllowed(const FRotator& Rotation) const
	{
		if (!bEnforceMatchingRotation)
		{
			return true;
		}

		const float Roll  = FRotator::NormalizeAxis(Rotation.Roll);
		const float Pitch = FRotator::NormalizeAxis(Rotation.Pitch);
		const float Yaw   = FRotator::NormalizeAxis(Rotation.Yaw);

		return Roll  >= MinimumMatchingRotation.Roll  - UE_KINDA_SMALL_NUMBER &&
			   Roll  <= MaximumMatchingRotation.Roll  + UE_KINDA_SMALL_NUMBER &&
			   Pitch >= MinimumMatchingRotation.Pitch - UE_KINDA_SMALL_NUMBER &&
			   Pitch <= MaximumMatchingRotation.Pitch + UE_KINDA_SMALL_NUMBER &&
			   Yaw   >= MinimumMatchingRotation.Yaw   - UE_KINDA_SMALL_NUMBER &&
			   Yaw   <= MaximumMatchingRotation.Yaw   + UE_KINDA_SMALL_NUMBER;
	}
	
	/**
	 * Matching-interval overload that accepts already-normalized axis values, skipping the internal normalization step.
	 * @param NormalizedRoll Normalized roll component (expected in [-180, 180)).
	 * @param NormalizedPitch Normalized pitch component.
	 * @param NormalizedYaw Normalized yaw component.
	 */
	bool IsMatchingRotationAllowed(const float NormalizedRoll, const float NormalizedPitch, const float NormalizedYaw) const
	{
		if (!bEnforceMatchingRotation)
		{
			return true;
		}

		return NormalizedRoll  >= MinimumMatchingRotation.Roll  - UE_KINDA_SMALL_NUMBER &&
			   NormalizedRoll  <= MaximumMatchingRotation.Roll  + UE_KINDA_SMALL_NUMBER &&
			   NormalizedPitch >= MinimumMatchingRotation.Pitch - UE_KINDA_SMALL_NUMBER &&
			   NormalizedPitch <= MaximumMatchingRotation.Pitch + UE_KINDA_SMALL_NUMBER &&
			   NormalizedYaw   >= MinimumMatchingRotation.Yaw   - UE_KINDA_SMALL_NUMBER &&
			   NormalizedYaw   <= MaximumMatchingRotation.Yaw   + UE_KINDA_SMALL_NUMBER;
	}
	
	/**
	 * Tests whether Rotation (treated as a delta between two rotations) lies within the difference interval.
	 * @param Rotation Delta rotation to check (axes will be normalized internally).
	 */
	bool IsDifferenceRotationAllowed(const FRotator& Rotation) const
	{
		if (!bEnforceDifferenceRotation)
		{
			return true;
		}

		const float Roll  = FRotator::NormalizeAxis(Rotation.Roll);
		const float Pitch = FRotator::NormalizeAxis(Rotation.Pitch);
		const float Yaw   = FRotator::NormalizeAxis(Rotation.Yaw);

		return Roll  >= MinimumDifferenceRotation.Roll  - UE_KINDA_SMALL_NUMBER &&
			   Roll  <= MaximumDifferenceRotation.Roll  + UE_KINDA_SMALL_NUMBER &&
			   Pitch >= MinimumDifferenceRotation.Pitch - UE_KINDA_SMALL_NUMBER &&
			   Pitch <= MaximumDifferenceRotation.Pitch + UE_KINDA_SMALL_NUMBER &&
			   Yaw   >= MinimumDifferenceRotation.Yaw   - UE_KINDA_SMALL_NUMBER &&
			   Yaw   <= MaximumDifferenceRotation.Yaw   + UE_KINDA_SMALL_NUMBER;
	}
	
	/**
	 * Difference-interval overload that accepts already-normalized axis values, skipping the internal normalization step.
	 * @param NormalizedRoll Normalized roll component of the delta.
	 * @param NormalizedPitch Normalized pitch component of the delta.
	 * @param NormalizedYaw Normalized yaw component of the delta.
	 */
	bool IsDifferenceRotationAllowed(const float NormalizedRoll, const float NormalizedPitch, const float NormalizedYaw) const
	{
		if (!bEnforceDifferenceRotation)
		{
			return true;
		}

		return NormalizedRoll  >= MinimumDifferenceRotation.Roll  - UE_KINDA_SMALL_NUMBER &&
			   NormalizedRoll  <= MaximumDifferenceRotation.Roll  + UE_KINDA_SMALL_NUMBER &&
			   NormalizedPitch >= MinimumDifferenceRotation.Pitch - UE_KINDA_SMALL_NUMBER &&
			   NormalizedPitch <= MaximumDifferenceRotation.Pitch + UE_KINDA_SMALL_NUMBER &&
			   NormalizedYaw   >= MinimumDifferenceRotation.Yaw   - UE_KINDA_SMALL_NUMBER &&
			   NormalizedYaw   <= MaximumDifferenceRotation.Yaw   + UE_KINDA_SMALL_NUMBER;
	}
};
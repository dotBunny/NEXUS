// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRotationConstraints.generated.h"



USTRUCT(BlueprintType)
struct NEXUSCORE_API FNRotationConstraints
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Matching", DisplayName = "Enforce Matching?")
	bool bEnforceMatchingRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Matching", DisplayName = "Minimum Matching Rotation", meta=(EditCondition="bEnforceMatchingRotation", EditConditionHides))
	FRotator MinimumMatchingRotation = FRotator(0.f, -180.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Matching", DisplayName = "Maximum Matching Rotation", meta=(EditCondition="bEnforceMatchingRotation", EditConditionHides))
	FRotator MaximumMatchingRotation = FRotator(0.f, 180.f, 0.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difference", DisplayName = "Enforce Difference?")
	bool bEnforceDifferenceRotation = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difference", DisplayName = "Minimum Difference Rotation", meta=(EditCondition="bEnforceDifferenceRotation", EditConditionHides))
	FRotator MinimumDifferenceRotation  = FRotator(0.f, -180.f, 0.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difference", DisplayName = "Maximum Difference Rotation", meta=(EditCondition="bEnforceDifferenceRotation", EditConditionHides))
	FRotator MaximumDifferenceRotation = FRotator(0.f, 180.f, 0.f);

	bool CopyTo(FNRotationConstraints& Other) const
	{
		Other = *this;
		return true;
	}

	bool IsEqual(const FNRotationConstraints& Other) const
	{
		return  bEnforceMatchingRotation == Other.bEnforceMatchingRotation &&
				bEnforceDifferenceRotation == Other.bEnforceDifferenceRotation &&
				MinimumMatchingRotation == Other.MinimumMatchingRotation &&
				MaximumMatchingRotation == Other.MaximumMatchingRotation &&
				MinimumDifferenceRotation == Other.MinimumDifferenceRotation &&
				MaximumDifferenceRotation == Other.MaximumDifferenceRotation;
	}

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
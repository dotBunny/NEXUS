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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Matching", DisplayName = "Minimum Matching Rotation")
	FRotator MinimumMatchingRotation = FRotator(0.f, -180.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Matching", DisplayName = "Maximum Matching Rotation")
	FRotator MaximumMatchingRotation = FRotator(0.f, 180.f, 0.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difference", DisplayName = "Enforce Difference?")
	bool bEnforceDifferenceRotation = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difference", DisplayName = "Minimum Difference Rotation")
	FQuat MinimumDifferenceRotation  = FQuat(0.f,0.f,0.f, 0.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difference", DisplayName = "Maximum Difference Rotation")
	FQuat MaximumDifferenceRotation = FQuat(0.f,0.f,180.f, 0.f);

	bool CopyTo(FNRotationConstraints& Other) const
	{
		Other = *this;
		return true;
	}

	bool IsEqual(const FNRotationConstraints& Other) const
	{
		return  MinimumMatchingRotation == Other.MinimumMatchingRotation &&
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

		return Roll  >= MinimumMatchingRotation.Roll  - KINDA_SMALL_NUMBER &&
			   Roll  <= MaximumMatchingRotation.Roll  + KINDA_SMALL_NUMBER &&
			   Pitch >= MinimumMatchingRotation.Pitch - KINDA_SMALL_NUMBER &&
			   Pitch <= MaximumMatchingRotation.Pitch + KINDA_SMALL_NUMBER &&
			   Yaw   >= MinimumMatchingRotation.Yaw   - KINDA_SMALL_NUMBER &&
			   Yaw   <= MaximumMatchingRotation.Yaw   + KINDA_SMALL_NUMBER;
	}
	
	bool IsMatchingRotationAllowed(const float NormalizedRoll, const float NormalizedPitch, const float NormalizedYaw) const
	{
		if (!bEnforceMatchingRotation)
		{
			return true;
		}

		return NormalizedRoll  >= MinimumMatchingRotation.Roll  - KINDA_SMALL_NUMBER &&
			   NormalizedRoll  <= MaximumMatchingRotation.Roll  + KINDA_SMALL_NUMBER &&
			   NormalizedPitch >= MinimumMatchingRotation.Pitch - KINDA_SMALL_NUMBER &&
			   NormalizedPitch <= MaximumMatchingRotation.Pitch + KINDA_SMALL_NUMBER &&
			   NormalizedYaw   >= MinimumMatchingRotation.Yaw   - KINDA_SMALL_NUMBER &&
			   NormalizedYaw   <= MaximumMatchingRotation.Yaw   + KINDA_SMALL_NUMBER;
	}
};
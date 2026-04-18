// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRotationConstraints.generated.h"



USTRUCT(BlueprintType)
struct NEXUSCORE_API FNRotationConstraints
{
	GENERATED_BODY()

	// TODO: Replace with MinimumMatchingRotation.Roll & MinimumMatchingRotation.Roll based constraint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Legacy", DisplayName = "Allow Roll (X)")
	bool bRoll = false;

	// TODO: Replace with MinimumMatchingRotation.Roll & MinimumMatchingRotation.Roll based constraint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Legacy", DisplayName = "Allow Pitch (Y)")
	bool bPitch = false;
	
	// TODO: Replace with MinimumMatchingRotation.Roll & MinimumMatchingRotation.Roll based constraint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Legacy", DisplayName = "Allow Yaw (Z)")
	bool bYaw = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Matching", DisplayName = "Enforce Matching?")
	bool bEnforceMatchingRotation = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Matching", DisplayName = "Minimum Matching Rotation")
	FQuat MinimumMatchingRotation = FQuat(0.f,0.f,0.f, 0.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Matching", DisplayName = "Maximum Matching Rotation")
	FQuat MaximumMatchingRotation = FQuat(0.f,0.f,180.f, 0.f);
	
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
		return  bRoll == Other.bRoll && bPitch == Other.bPitch && bYaw == Other.bYaw &&
				
				MinimumMatchingRotation == Other.MinimumMatchingRotation &&
				MaximumMatchingRotation == Other.MaximumMatchingRotation &&
				MinimumDifferenceRotation == Other.MinimumDifferenceRotation &&
				MaximumDifferenceRotation == Other.MaximumDifferenceRotation;
	}
};
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRotationConstraint.generated.h"

USTRUCT(BlueprintType)
struct NEXUSCORE_API FNRotationConstraint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Allow Roll (X)")
	bool bRoll = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Allow Pitch (Y)")
	bool bPitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Allow Yaw (Z)")
	bool bYaw = true;

	bool CopyTo(FNRotationConstraint& Other) const
	{
		Other = *this;
		return true;
	}

	bool IsEqual(const FNRotationConstraint& Other) const
	{
		return  bRoll == Other.bRoll && bPitch == Other.bPitch && bYaw == Other.bYaw;
	}
};
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRotationConstraints.generated.h"

USTRUCT(BlueprintType)
struct NEXUSCORE_API FNRotationConstraints
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Allow Roll (X)")
	bool bRoll = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Allow Pitch (Y)")
	bool bPitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Allow Yaw (Z)")
	bool bYaw = true;

	bool CopyTo(FNRotationConstraints& Other) const
	{
		Other = *this;
		return true;
	}

	bool IsEqual(const FNRotationConstraints& Other) const
	{
		return  bRoll == Other.bRoll && bPitch == Other.bPitch && bYaw == Other.bYaw;
	}
	
	bool IsAll() const
	{
		return bYaw && bPitch && bRoll;
	}
	
	bool IsNone() const
	{
		return !bYaw && !bPitch && !bRoll;
	}
};
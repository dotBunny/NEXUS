// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCardinalDirection.h"
#include "NCardinalRotation.generated.h"

USTRUCT(BlueprintType)
struct NEXUSCORE_API FNCardinalRotation
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENCardinalDirection Roll = ENCardinalDirection::North;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENCardinalDirection Pitch = ENCardinalDirection::North;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENCardinalDirection Yaw = ENCardinalDirection::North;

	bool CopyTo(FNCardinalRotation& Other) const
	{
		Other = *this;
		return true;
	}

	FRotator ToRotator() const
	{
		FRotator Result;
		
		Result.Roll = FNCardinalDirectionUtils::ToDecimalDegrees(Roll);
		Result.Pitch = FNCardinalDirectionUtils::ToDecimalDegrees(Pitch);
		Result.Yaw = FNCardinalDirectionUtils::ToDecimalDegrees(Yaw);
		
		return Result;
	}

	FRotator ToRotatorNormalized() const
	{
		FRotator Result;
		
		Result.Roll = FNCardinalDirectionUtils::ToDecimalDegreesNormalized(Roll);
		Result.Pitch = FNCardinalDirectionUtils::ToDecimalDegreesNormalized(Pitch);
		Result.Yaw = FNCardinalDirectionUtils::ToDecimalDegreesNormalized(Yaw);
		
		return Result;
	}

	FVector ToVector() const
	{
		return FVector(
			FNCardinalDirectionUtils::ToDecimalDegrees(Roll),
			FNCardinalDirectionUtils::ToDecimalDegrees(Pitch),
			FNCardinalDirectionUtils::ToDecimalDegrees(Yaw));
	}

	FVector ToVectorNormalized() const
	{
		return FVector(
			FNCardinalDirectionUtils::ToDecimalDegreesNormalized(Roll),
			FNCardinalDirectionUtils::ToDecimalDegreesNormalized(Pitch),
			FNCardinalDirectionUtils::ToDecimalDegreesNormalized(Yaw));
	}
	
	bool IsEqual(const FNCardinalRotation& Other) const
	{
		return Roll == Other.Roll &&
			Pitch == Other.Pitch &&
			Yaw == Other.Yaw;
	}

	static FNCardinalRotation CreateFromNormalized(const FRotator& InRotator)
	{
		FNCardinalRotation Result;
		
		Result.Roll = FNCardinalDirectionUtils::ToCardinalDirectionNormalized(InRotator.Roll);
		Result.Pitch = FNCardinalDirectionUtils::ToCardinalDirectionNormalized(InRotator.Pitch);
		Result.Yaw = FNCardinalDirectionUtils::ToCardinalDirectionNormalized(InRotator.Yaw);
		
		return Result;
		
	}
};
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
	FIntVector3 ToIntVector3()
	{
// #SONARQUBE-DISABLE-CPP_S7035 C++ 23 feature to use to_underlying	
		return FIntVector3(static_cast<uint8>(Roll), static_cast<uint8>(Pitch), static_cast<uint8>(Yaw));
// #SONARQUBE-ENABLE-CPP_S7035 C++ 23 feature to use to_underlying	
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

	
	static FNCardinalRotation CreateFrom(const FRotator& InRotator)
	{
		FNCardinalRotation Result;
		
		Result.Roll = FNCardinalDirectionUtils::ToCardinalDirection(InRotator.Roll);
		Result.Pitch = FNCardinalDirectionUtils::ToCardinalDirection(InRotator.Pitch);
		Result.Yaw = FNCardinalDirectionUtils::ToCardinalDirection(InRotator.Yaw);
		
		return Result;
	}
	
	static FNCardinalRotation CreateFromNormalized(const FRotator& InRotator)
	{
		FNCardinalRotation Result;
		
		Result.Roll = FNCardinalDirectionUtils::ToCardinalDirectionNormalized(InRotator.Roll);
		Result.Pitch = FNCardinalDirectionUtils::ToCardinalDirectionNormalized(InRotator.Pitch);
		Result.Yaw = FNCardinalDirectionUtils::ToCardinalDirectionNormalized(InRotator.Yaw);
		
		return Result;
	}
	
	
	static FVector GetUnitSize(const FNCardinalRotation& CardinalDirection, const float UnitSizeX, const float UnitSizeY)
	{
		const FRotator Rotation = CardinalDirection.ToRotatorNormalized();
		const FVector PreRotatedSize = FVector(0.f, UnitSizeX, UnitSizeY);
		const FVector RotatedSize = Rotation.RotateVector(PreRotatedSize);
		return RotatedSize;
	}
};
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCardinalDirection.h"
#include "NCardinalRotation.generated.h"


/**
 * Rotation expressed as three ENCardinalDirection components (Roll, Pitch, Yaw).
 *
 * Useful for grid-aligned gameplay that only needs 16-wind orientations. Conversion helpers
 * produce either unsigned [0, 360) or normalized [-180, 180) degree representations.
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNCardinalRotation
{
	GENERATED_BODY()

	/** Roll component, in cardinal steps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENCardinalDirection Roll = ENCardinalDirection::North;

	/** Pitch component, in cardinal steps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENCardinalDirection Pitch = ENCardinalDirection::North;

	/** Yaw component, in cardinal steps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENCardinalDirection Yaw = ENCardinalDirection::North;

	/**
	 * Copies this rotation to Other.
	 * @param Other Destination instance.
	 * @return Always true; the bool return mirrors the IO-style helpers elsewhere in the codebase.
	 */
	bool CopyTo(FNCardinalRotation& Other) const
	{
		Other = *this;
		return true;
	}

	/** Packs the three cardinal components into an FIntVector3 of underlying enum values. */
	FIntVector3 ToIntVector3()
	{
// #SONARQUBE-DISABLE-CPP_S7035 C++ 23 feature to use to_underlying
		return FIntVector3(static_cast<uint8>(Roll), static_cast<uint8>(Pitch), static_cast<uint8>(Yaw));
// #SONARQUBE-ENABLE-CPP_S7035 C++ 23 feature to use to_underlying
	}

	/** Converts this cardinal rotation to an unsigned-degree FRotator. */
	FRotator ToRotator() const
	{
		FRotator Result;
		
		Result.Roll = FNCardinalDirectionUtils::ToDecimalDegrees(Roll);
		Result.Pitch = FNCardinalDirectionUtils::ToDecimalDegrees(Pitch);
		Result.Yaw = FNCardinalDirectionUtils::ToDecimalDegrees(Yaw);
		
		return Result;
	}

	/** Converts this cardinal rotation to a signed-degree FRotator (values in [-180, 180)). */
	FRotator ToRotatorNormalized() const
	{
		FRotator Result;
		
		Result.Roll = FNCardinalDirectionUtils::ToDecimalDegreesNormalized(Roll);
		Result.Pitch = FNCardinalDirectionUtils::ToDecimalDegreesNormalized(Pitch);
		Result.Yaw = FNCardinalDirectionUtils::ToDecimalDegreesNormalized(Yaw);
		
		return Result;
	}

	/** Packs the three components' unsigned-degree bearings into an FVector. */
	FVector ToVector() const
	{
		return FVector(
			FNCardinalDirectionUtils::ToDecimalDegrees(Roll),
			FNCardinalDirectionUtils::ToDecimalDegrees(Pitch),
			FNCardinalDirectionUtils::ToDecimalDegrees(Yaw));
	}

	/** Packs the three components' signed-degree bearings into an FVector. */
	FVector ToVectorNormalized() const
	{
		return FVector(
			FNCardinalDirectionUtils::ToDecimalDegreesNormalized(Roll),
			FNCardinalDirectionUtils::ToDecimalDegreesNormalized(Pitch),
			FNCardinalDirectionUtils::ToDecimalDegreesNormalized(Yaw));
	}

	/**
	 * Component-wise equality test.
	 * @param Other Right-hand side of the comparison.
	 */
	bool IsEqual(const FNCardinalRotation& Other) const
	{
		return Roll == Other.Roll &&
			Pitch == Other.Pitch &&
			Yaw == Other.Yaw;
	}

	/**
	 * Builds an FNCardinalRotation by snapping each component of InRotator (unsigned degrees) to its nearest cardinal.
	 * @param InRotator Unsigned-degree rotation to snap.
	 */
	static FNCardinalRotation CreateFrom(const FRotator& InRotator)
	{
		FNCardinalRotation Result;
		
		Result.Roll = FNCardinalDirectionUtils::ToCardinalDirection(InRotator.Roll);
		Result.Pitch = FNCardinalDirectionUtils::ToCardinalDirection(InRotator.Pitch);
		Result.Yaw = FNCardinalDirectionUtils::ToCardinalDirection(InRotator.Yaw);
		
		return Result;
	}
	
	/**
	 * Builds an FNCardinalRotation by snapping each component of InRotator (signed degrees) to its nearest cardinal.
	 * @param InRotator Signed-degree rotation to snap.
	 */
	static FNCardinalRotation CreateFromNormalized(const FRotator& InRotator)
	{
		FNCardinalRotation Result;
		
		Result.Roll = FNCardinalDirectionUtils::ToCardinalDirectionNormalized(InRotator.Roll);
		Result.Pitch = FNCardinalDirectionUtils::ToCardinalDirectionNormalized(InRotator.Pitch);
		Result.Yaw = FNCardinalDirectionUtils::ToCardinalDirectionNormalized(InRotator.Yaw);
		
		return Result;
	}
	
	
	/**
	 * Rotates a 2D unit-size footprint (0, UnitSizeX, UnitSizeY) by CardinalDirection.
	 * @param CardinalDirection The cardinal rotation to apply.
	 * @param UnitSizeX Size along the local Y axis before rotation.
	 * @param UnitSizeY Size along the local Z axis before rotation.
	 * @return The footprint vector after applying the rotation.
	 */
	static FVector GetUnitSize(const FNCardinalRotation& CardinalDirection, const float UnitSizeX, const float UnitSizeY)
	{
		const FRotator Rotation = CardinalDirection.ToRotatorNormalized();
		const FVector PreRotatedSize = FVector(0.f, UnitSizeX, UnitSizeY);
		const FVector RotatedSize = Rotation.RotateVector(PreRotatedSize);
		return RotatedSize;
	}
};
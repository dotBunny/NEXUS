// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NBoxPicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NBoxPickerLibrary.generated.h"

UCLASS()
class NEXUSPICKER_API UNBoxPickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Next Point Inside", Category = "NEXUS|Picker")
	static FVector BoxNextPointInside(const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::NextPointInside(ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Next Grounded Point Inside", Category = "NEXUS|Picker")
	static FVector BoxNextGroundedPointInside(const FVector& Origin, const FBox& Dimensions,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNBoxPicker::NextGroundedPointInside(ReturnLocation, Origin, Dimensions, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random Point Inside", Category = "NEXUS|Picker")
	static FVector BoxRandomPointInside(const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomPointInside(ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random Grounded Point Inside", Category = "NEXUS|Picker")
	static FVector BoxRandomGroundedPointInside(const FVector& Origin, const FBox& Dimensions,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomGroundedPointInside(ReturnLocation, Origin, Dimensions, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random One-Shot Point Inside", Category = "NEXUS|Picker")
	static FVector BoxRandomOneShotPointInside(const int32 Seed, const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomOneShotPointInside(Seed, ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random One-Shot Grounded Point Inside", Category = "NEXUS|Picker")
	static FVector BoxRandomOneShotGroundedPointInside(const int32 Seed, const FVector& Origin, const FBox& Dimensions,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomOneShotGroundedPointInside(Seed, ReturnLocation, Origin, Dimensions, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;	
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random Tracked Point Inside", Category = "NEXUS|Picker")
	static FVector BoxRandomTrackedPointInside(int32& Seed, const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomOneShotPointInside(Seed, ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random Tracked Grounded Point Inside", Category = "NEXUS|Picker")
	static FVector BoxRandomTrackedGroundedPointInside(int32& Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomTrackedGroundedPointInside(Seed, ReturnLocation, Origin, Dimensions, InWorld, CastBuffer, CollisionChannel);
		
		return ReturnLocation;
	}
};
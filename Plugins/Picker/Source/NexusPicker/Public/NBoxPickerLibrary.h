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

	UFUNCTION(BlueprintCallable, DisplayName = "Next Point Inside", Category = "NEXUS|Picker|Box")
	static FVector NextPointInside(const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::NextPointInside(ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Next Grounded Point Inside", Category = "NEXUS|Picker|Box")
	static FVector NextGroundedPointInside(const FVector& Origin, const FBox& Dimensions,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNBoxPicker::NextGroundedPointInside(ReturnLocation, Origin, Dimensions, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Point Inside", Category = "NEXUS|Picker|Box")
	static FVector RandomPointInside(const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomPointInside(ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Grounded Point Inside", Category = "NEXUS|Picker|Box")
	static FVector RandomGroundedPointInside(const FVector& Origin, const FBox& Dimensions,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomGroundedPointInside(ReturnLocation, Origin, Dimensions, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random One-Shot Point Inside", Category = "NEXUS|Picker|Box")
	static FVector RandomOneShotPointInside(const int32 Seed, const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomOneShotPointInside(Seed, ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random One-Shot Grounded Point Inside", Category = "NEXUS|Picker|Box")
	static FVector RandomOneShotGroundedPointInside(const int32 Seed, const FVector& Origin, const FBox& Dimensions,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomOneShotGroundedPointInside(Seed, ReturnLocation, Origin, Dimensions, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;	
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random Tracked Point Inside", Category = "NEXUS|Picker|Box")
	static FVector RandomTrackedPointInside(int32& Seed, const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomOneShotPointInside(Seed, ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Tracked Grounded Point Inside", Category = "NEXUS|Picker|Box")
	static FVector RandomTrackedGroundedPointInside(int32& Seed, const FVector& Origin, const FBox& Dimensions,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomTrackedGroundedPointInside(Seed, ReturnLocation, Origin, Dimensions, InWorld, CastBuffer, CollisionChannel);
		
		return ReturnLocation;
	}
};
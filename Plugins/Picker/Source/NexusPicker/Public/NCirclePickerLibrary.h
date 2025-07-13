// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCirclePicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NCirclePickerLibrary.generated.h"

UCLASS()
class NEXUSPICKER_API UNCirclePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, DisplayName = "Next Point Inside", Category = "NEXUS|Picker|Circle")
	static FVector NextPointInside(const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNCirclePicker::NextPointInside(ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Next Grounded Point Inside", Category = "NEXUS|Picker|Circle")
	static FVector NextGroundedPointInside(const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::NextGroundedPointInside(ReturnLocation, Origin, MinimumRadius, MaximumRadius, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Point Inside", Category = "NEXUS|Picker|Circle")
	static FVector RandomPointInside(const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomPointInside(ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Grounded Point Inside", Category = "NEXUS|Picker|Circle")
	static FVector RandomGroundedPointInside(const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomGroundedPointInside(ReturnLocation, Origin, MinimumRadius, MaximumRadius, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random One-Shot Point Inside", Category = "NEXUS|Picker|Circle")
	static FVector RandomOneShotPointInside(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomOneShotPointInside(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random One-Shot Grounded Point Inside", Category = "NEXUS|Picker|Circle")
	static FVector RandomOneShotGroundedPointInside(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomOneShotGroundedPointInside(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;	
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random Tracked Point Inside", Category = "NEXUS|Picker|Circle")
	static FVector RandomTrackedPointInside(int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomOneShotPointInside(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Tracked Grounded Point Inside", Category = "NEXUS|Picker|Circle")
	static FVector RandomTrackedGroundedPointInside(int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomTrackedGroundedPointInside(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, InWorld, CastBuffer, CollisionChannel);
		
		return ReturnLocation;
	}
};
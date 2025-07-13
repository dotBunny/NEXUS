// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSpherePicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NSpherePickerLibrary.generated.h"

UCLASS()
class NEXUSPICKER_API UNSpherePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, DisplayName = "Next Point Inside", Category = "NEXUS|Picker|Sphere")
	static FVector NextPointInside(const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::NextPointInside(ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Next Grounded Point Inside", Category = "NEXUS|Picker|Sphere")
	static FVector NextGroundedPointInside(const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::NextGroundedPointInside(ReturnLocation, Origin, MinimumRadius, MaximumRadius, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Point Inside", Category = "NEXUS|Picker|Sphere")
	static FVector RandomPointInside(const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomPointInside(ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Grounded Point Inside", Category = "NEXUS|Picker|Sphere")
	static FVector RandomGroundedPointInside(const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomGroundedPointInside(ReturnLocation, Origin, MinimumRadius, MaximumRadius, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random One-Shot Point Inside", Category = "NEXUS|Picker|Sphere")
	static FVector RandomOneShotPointInside(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomOneShotPointInside(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random One-Shot Grounded Point Inside", Category = "NEXUS|Picker|Sphere")
	static FVector RandomOneShotGroundedPointInside(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomOneShotGroundedPointInside(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;	
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random Tracked Point Inside", Category = "NEXUS|Picker|Sphere")
	static FVector RandomTrackedPointInside(int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomOneShotPointInside(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Tracked Grounded Point Inside", Category = "NEXUS|Picker|Sphere")
	static FVector RandomTrackedGroundedPointInside(int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomTrackedGroundedPointInside(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, InWorld, CastBuffer, CollisionChannel);
		
		return ReturnLocation;
	}
};
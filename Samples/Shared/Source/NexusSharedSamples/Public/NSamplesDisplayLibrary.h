// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NSamplesDisplayLibrary.generated.h"

class ANSamplesDisplayActor;

namespace NEXUS::Samples
{
	constexpr float TimerDrawThickness = 1.f;
}

/**
 * @class UNSamplesDisplayLibrary
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS: Samples Display Library")
class UNSamplesDisplayLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "NEXUS|Samples Display", DisplayName=" Draw Point")
	static void TimerDrawPoint(ANSamplesDisplayActor* SamplesDisplay, const FVector Location, const int TimerIntervals = 1);
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Samples Display", DisplayName="Draw Sphere")
	static void TimerDrawSphere(ANSamplesDisplayActor* SamplesDisplay, const FVector Location, const float Radius, const int TimerIntervals = 1);
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Samples Display", DisplayName="Draw Combo Sphere")
	static void TimerDrawComboSphere(ANSamplesDisplayActor* SamplesDisplay, const FVector Location, const FVector2D InnerOuter, const int TimerIntervals = 1);
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Samples Display", DisplayName="Draw Box")
	static void TimerDrawBox(ANSamplesDisplayActor* SamplesDisplay, const FVector Location, const FBox& Dimensions, const int TimerIntervals = 1);
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Samples Display", DisplayName="Draw Combo Box")
	static void TimerDrawComboBox(ANSamplesDisplayActor* SamplesDisplay, const FVector Location, const FBox& InnerDimensions, const FBox& OuterDimensions, const int TimerIntervals = 1);
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Samples Display", DisplayName="Draw Circle")
	static void TimerDrawCircle(ANSamplesDisplayActor* SamplesDisplay, const FVector Location, const float& Radius, const FRotator& Rotation = FRotator::ZeroRotator, const int TimerIntervals = 1);
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Samples Display", DisplayName="Draw Combo Circle")
	static void TimerDrawComboCircle(ANSamplesDisplayActor* SamplesDisplay, const FVector Location, const FVector2D& InnerOuter, const FRotator& Rotation = FRotator::ZeroRotator, const int TimerIntervals = 1);
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Samples Display", DisplayName="Draw Spline")
	static void TimerDrawSpline(ANSamplesDisplayActor* SamplesDisplay, const USplineComponent* Spline, const int TimerIntervals = 1);
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Samples Display", DisplayName="Draw Rectangle")
	static void TimerDrawRectangle(ANSamplesDisplayActor* SamplesDisplay, const FVector Location, const FVector2D& Dimensions, const FRotator& Rotation = FRotator::ZeroRotator, int TimerIntervals = 1) ;
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false,  Category = "NEXUS|Samples Display", DisplayName="Draw Combo Rectangle")
	static void TimerDrawComboRectangle(ANSamplesDisplayActor* SamplesDisplay, const FVector Location, const FVector2D& InnerDimensions, const FVector2D& OuterDimensions, const FRotator& Rotation = FRotator::ZeroRotator, const int TimerIntervals = 1);

private:
	static FMatrix BaseDrawMatrix;
};
#pragma once

#include "NPositionRotation.generated.h"

USTRUCT(BlueprintType)
struct FNPositionRotation
{
	GENERATED_BODY()
	
	UPROPERTY()
	FVector Position = FVector::ZeroVector;
	
	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;
};

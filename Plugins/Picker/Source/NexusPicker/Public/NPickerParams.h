// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.generated.h"

USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNPickerParams
{
	GENERATED_BODY()	
	
	UPROPERTY(Category = "Base", BlueprintReadWrite)
	int Count = 1;
	
	UPROPERTY(Category = "Base", BlueprintReadWrite)
	FVector Origin = FVector::ZeroVector;
	
	UPROPERTY(Category = "Base", BlueprintReadWrite)
	int32 Seed = -1;
	
	//bool bOnExtents = false;
	
	UPROPERTY(Category = "Projection", BlueprintReadWrite)
	bool bProjectPoint = false;
	
	UPROPERTY(Category = "Projection", BlueprintReadWrite)
	TObjectPtr<UObject> WorldContextObject = nullptr;
	
	UPROPERTY(Category = "Projection", BlueprintReadWrite)
	FVector Projection = FVector(0,0,-500.f);
	
	UPROPERTY(Category = "Projection", BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> ProjectionCollisionChannel = ECC_WorldStatic;
};
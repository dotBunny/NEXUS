// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Types/NCardinalRotation.h"
#include "NCellJunctionDetails.generated.h"

UENUM()
enum class ENCellJunctionType : uint8
{
	NCJT_TwoWaySocket = 0 UMETA(DisplayName = "Two Way Socket"),
	NCJT_OneWaySocket = 1 UMETA(DisplayName = "One Way Socket"),
};

USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNCellJunctionDetails
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly)
	ENCellJunctionType Type = ENCellJunctionType::NCJT_TwoWaySocket;

	UPROPERTY(EditInstanceOnly)
	FIntVector2 UnitSize = FIntVector2(4, 2);
	
	UPROPERTY(VisibleAnywhere)
	FVector RootRelativeLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere)
	FNCardinalRotation RootRelativeCardinalRotation;

	UPROPERTY(VisibleAnywhere)
	int32 InstanceIdentifier = -1;
	
	bool CopyTo(FNCellJunctionDetails& Other) const
	{
		Other = *this;
		return true;
	}

	bool IsEqual(const FNCellJunctionDetails& Other) const
	{
		return
			InstanceIdentifier == Other.InstanceIdentifier &&
			Type == Other.Type &&
			UnitSize == Other.UnitSize &&
			RootRelativeLocation == Other.RootRelativeLocation &&
			RootRelativeCardinalRotation.IsEqual(Other.RootRelativeCardinalRotation);
	}
};


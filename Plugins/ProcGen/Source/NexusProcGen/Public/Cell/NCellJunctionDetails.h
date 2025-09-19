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

// Maybe sockets should be 3d? so its a plug type nature so something cane blend better?
UENUM()
enum class ENCellJunctionSize : uint8
{
	NCJS_1x1 = 0 UMETA(DisplayName = "1x1"),
	NCJS_1x2 = 1 UMETA(DisplayName = "1x2"),
	NCJS_1x3 = 2 UMETA(DisplayName = "1x3"),
	NCJS_1x4 = 3 UMETA(DisplayName = "1x4"),
	NCJS_2x1 = 4 UMETA(DisplayName = "2x1"),
	NCJS_2x3 = 5 UMETA(DisplayName = "2x3"),
	NCJS_2x4 = 6 UMETA(DisplayName = "2x4"),
	NCJS_3x1 = 7 UMETA(DisplayName = "3x1"),
	NCJS_3x2 = 8 UMETA(DisplayName = "3x2"),
	NCJS_3x3 = 9 UMETA(DisplayName = "3x3"),
	NCJS_3x4 = 10 UMETA(DisplayName = "3x4"),
	NCJS_4x1 = 11 UMETA(DisplayName = "4x1"),
	NCJS_4x2 = 12 UMETA(DisplayName = "4x2"),
	NCJS_4x3 = 13 UMETA(DisplayName = "4x3"),
	NCJS_4x4 = 14 UMETA(DisplayName = "4x4"),
};

USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNCellJunctionDetails
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly)
	ENCellJunctionType Type = ENCellJunctionType::NCJT_TwoWaySocket;

	UPROPERTY(EditInstanceOnly)
	ENCellJunctionSize Size = ENCellJunctionSize::NCJS_1x1;
	
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
			Size == Other.Size &&
			RootRelativeLocation == Other.RootRelativeLocation &&
			RootRelativeCardinalRotation.IsEqual(Other.RootRelativeCardinalRotation);
	}
};


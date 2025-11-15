// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NArrayUtils.h"
#include "NCellJunctionBlockerActor.h"
#include "NCellVoxelData.h"
#include "Math/NVoxelUtils.h"
#include "Types/NCardinalRotation.h"
#include "NCellJunctionDetails.generated.h"

UENUM()
enum class ENCellJunctionType : uint8
{
	CJT_TwoWaySocket = 0 UMETA(DisplayName = "Two Way Socket"),
	CJT_OneWaySocket = 1 UMETA(DisplayName = "One Way Socket"),
};

UENUM()
enum class ENCellJunctionRequirements : uint8
{
	CJR_Required = 0 UMETA(DisplayName = "Required", ToolTip = "This junction must have an associated junction socketed to it."),
	CJR_AllowBlocking = 1 UMETA(DisplayName = "Allow Blocking", ToolTip = "This junction can be filled with the components referenced blocker(s) instead of being linked to another junction."),
	CJR_AllowEmpty = 2 UMETA(DisplayName = "Allow Empty", ToolTip = "This junction can be left unfilled and not linked."),
};

USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNCellJunctionDetails
{
	GENERATED_BODY()
	
	UPROPERTY(EditInstanceOnly)
	ENCellJunctionType Type = ENCellJunctionType::CJT_TwoWaySocket;

	UPROPERTY(EditInstanceOnly)
	ENCellJunctionRequirements Requirements = ENCellJunctionRequirements::CJR_Required;

	UPROPERTY(EditAnywhere, meta = (EditCondition="Requirements==ENCellJunctionRequirements::CJR_AllowBlocking", ToolTip = "The blocker(s) that can be used to fill this junction.", EditConditionHides))
	TArray<TSubclassOf<ANCellJunctionBlockerActor>> Blockers;
	
	UPROPERTY(EditInstanceOnly)
	FIntVector2 UnitSize = FIntVector2(4, 2);
	
	UPROPERTY(EditInstanceOnly)
	FNVoxelCoordinate VoxelOrigin;
	
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
			InstanceIdentifier == Other.InstanceIdentifier
			&& Requirements == Other.Requirements
			&& Type == Other.Type
			&& UnitSize == Other.UnitSize
			&& RootRelativeLocation == Other.RootRelativeLocation
			&& FNArrayUtils::IsSameOrderedValues(Blockers, Other.Blockers)
			&& RootRelativeCardinalRotation.IsEqual(Other.RootRelativeCardinalRotation);
	}
};


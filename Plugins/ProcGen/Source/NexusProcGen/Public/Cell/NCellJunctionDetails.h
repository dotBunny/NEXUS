// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NArrayUtils.h"
#include "NCellJunctionBlockerActor.h"
#include "Types/NCardinalRotation.h"
#include "NCellJunctionDetails.generated.h"

UENUM()
enum class ENCellJunctionType : uint8
{
	TwoWaySocket = 0,
	OneWaySocket = 1,
};

UENUM()
enum class ENCellJunctionRequirements : uint8
{
	Required =		0 UMETA(DisplayName = "Required", ToolTip = "This junction must have an associated junction socketed to it."),
	AllowBlocking = 1 UMETA(DisplayName = "Allow Blocking", ToolTip = "This junction can be filled with the components referenced blocker(s) instead of being linked to another junction."),
	AllowEmpty =	2 UMETA(DisplayName = "Allow Empty", ToolTip = "This junction can be left unfilled and not linked."),
};

USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNCellJunctionDetails
{
	GENERATED_BODY()
	
	UPROPERTY(EditInstanceOnly)
	ENCellJunctionType Type = ENCellJunctionType::TwoWaySocket;

	UPROPERTY(EditInstanceOnly)
	ENCellJunctionRequirements Requirements = ENCellJunctionRequirements::Required;

	UPROPERTY(EditAnywhere, meta = (EditCondition="Requirements==ENCellJunctionRequirements::AllowBlocking", ToolTip = "The blocker(s) that can be used to fill this junction.", EditConditionHides))
	TArray<TSubclassOf<ANCellJunctionBlockerActor>> Blockers;
	
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
			InstanceIdentifier == Other.InstanceIdentifier
			&& Requirements == Other.Requirements
			&& Type == Other.Type
			&& UnitSize == Other.UnitSize
			&& RootRelativeLocation == Other.RootRelativeLocation
			&& FNArrayUtils::IsSameOrderedValues(Blockers, Other.Blockers)
			&& RootRelativeCardinalRotation.IsEqual(Other.RootRelativeCardinalRotation);
	}
};


// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NArrayUtils.h"
#include "NCellJunctionBlockerActor.h"
#include "NCellJunctionDetails.generated.h"

UENUM()
enum class ENCellJunctionType : uint8
{
	TwoWaySocket = 0 UMETA(DisplayName="Two-Way", ToolTip="Passage, accessible both directions."),
	InOnlySocket = 1 UMETA(DisplayName="In-Only", ToolTip="Inward passage only."),
	OutOnlySocket = 2 UMETA(DisplayName="Out-Only", ToolTip="Outward passage only."),
	OneWaySocket = 3 UMETA(DisplayName="One-Way", ToolTip="A passage can only happen once, in either direction.")
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
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	ENCellJunctionType Type = ENCellJunctionType::TwoWaySocket;

	UPROPERTY(EditInstanceOnly)
	ENCellJunctionRequirements Requirements = ENCellJunctionRequirements::Required;

	// TODO: this is not prolly gonna get used
	UPROPERTY(EditAnywhere, meta = (EditCondition="Requirements==ENCellJunctionRequirements::AllowBlocking", ToolTip = "The blocker(s) that can be used to fill this junction.", EditConditionHides))
	TArray<TSubclassOf<ANCellJunctionBlockerActor>> Blockers;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	FIntVector2 SocketSize = FIntVector2(2, 4);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector RootRelativeLocation = FVector::ZeroVector;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRotator RootRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere)
	int32 InstanceIdentifier = -1;
	
	UPROPERTY(VisibleAnywhere)
	bool bInsideHull = false;
	
	bool CopyTo(FNCellJunctionDetails& Other) const
	{
		Other = *this;
		return true;
	}

	bool IsEqual(const FNCellJunctionDetails& Other) const
	{
		return
			InstanceIdentifier == Other.InstanceIdentifier
			&& bInsideHull == Other.bInsideHull
			
			&& Requirements == Other.Requirements
			&& Type == Other.Type
			&& SocketSize == Other.SocketSize
			
			&& RootRelativeLocation == Other.RootRelativeLocation
			&& RootRelativeRotation == Other.RootRelativeRotation
			
			&& FNArrayUtils::IsSameOrderedValues(Blockers, Other.Blockers);
	}
};


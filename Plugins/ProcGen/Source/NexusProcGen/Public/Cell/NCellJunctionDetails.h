// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NArrayUtils.h"
#include "NCellJunctionBlockerActor.h"
#include "Types/NRotationConstraints.h"
#include "NCellJunctionDetails.generated.h"

/**
 * Directionality of a cell junction — constrains whether it can act as an entry, exit, or both during graph traversal.
 */
UENUM()
enum class ENCellJunctionType : uint8
{
	TwoWaySocket = 0 UMETA(DisplayName="Two-Way", ToolTip="Passage, accessible both directions."),
	InOnlySocket = 1 UMETA(DisplayName="In-Only", ToolTip="Inward passage only."),
	OutOnlySocket = 2 UMETA(DisplayName="Out-Only", ToolTip="Outward passage only."),
	OneWaySocket = 3 UMETA(DisplayName="One-Way", ToolTip="A passage can only happen once, in either direction.")
};

/**
 * How a junction may be satisfied during generation — required, optionally blocked, or allowed to remain empty.
 */
UENUM()
enum class ENCellJunctionRequirements : uint8
{
	Required =		0 UMETA(DisplayName = "Required", ToolTip = "This junction must have an associated junction socketed to it."),
	AllowBlocking = 1 UMETA(DisplayName = "Allow Blocking", ToolTip = "This junction can be filled with the components referenced blocker(s) instead of being linked to another junction."),
	AllowEmpty =	2 UMETA(DisplayName = "Allow Empty", ToolTip = "This junction can be left unfilled and not linked."),
};

/**
 * Persistent data describing a single junction on a cell.
 *
 * Stored both on UNCellJunctionComponent (live in the level) and in UNCell::Junctions (side-car asset)
 * so the two can be diffed and synced without loading the full level.
 */
USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNCellJunctionDetails
{
	GENERATED_BODY()
	
	/** Directionality of this junction. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	ENCellJunctionType Type = ENCellJunctionType::TwoWaySocket;

	/** Requirement satisfied during graph generation. */
	UPROPERTY(EditInstanceOnly)
	ENCellJunctionRequirements Requirements = ENCellJunctionRequirements::Required;

	// TODO: this is not prolly gonna get used
	UPROPERTY(EditAnywhere, meta = (EditCondition="Requirements==ENCellJunctionRequirements::AllowBlocking", ToolTip = "The blocker(s) that can be used to fill this junction.", EditConditionHides))
	TArray<TSubclassOf<ANCellJunctionBlockerActor>> Blockers;

	/** Size of the junction socket in grid units (width, height). */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	FIntVector2 SocketSize = FIntVector2(2, 4);

	/** World-space location of the junction — derived from the component transform. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector WorldLocation = FVector::ZeroVector;

	/** World-space rotation of the junction — derived from the component transform. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRotator WorldRotation = FRotator::ZeroRotator;

	/** Allowed rotations for this junction when the owning cell is placed by the generator. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	FNRotationConstraints RotationConstraints;

	/** Unique per-cell identifier; allocated by ANCellActor::GetCellJunctionNextIdentifier. */
	UPROPERTY(VisibleAnywhere)
	int32 InstanceIdentifier = -1;

	/** Derived: true if the junction point lies inside the cell's hull (set during UpdateHullDerivedData). */
	UPROPERTY(VisibleAnywhere)
	bool bInsideHull = false;

	/** Copy this struct into Other by value. @return Always true. */
	bool CopyTo(FNCellJunctionDetails& Other) const
	{
		Other = *this;
		return true;
	}

	/** @return true if every field matches structurally; used to detect author-time drift from the side-car. */
	bool IsEqual(const FNCellJunctionDetails& Other) const
	{
		return
			InstanceIdentifier == Other.InstanceIdentifier
			&& bInsideHull == Other.bInsideHull
			
			&& Requirements == Other.Requirements
			&& Type == Other.Type
			&& SocketSize == Other.SocketSize
			
			&& WorldLocation == Other.WorldLocation
			&& WorldRotation == Other.WorldRotation
			&& RotationConstraints.IsEqual(Other.RotationConstraints)
			
			&& FNArrayUtils::IsSameOrderedValues(Blockers, Other.Blockers);
	}
};


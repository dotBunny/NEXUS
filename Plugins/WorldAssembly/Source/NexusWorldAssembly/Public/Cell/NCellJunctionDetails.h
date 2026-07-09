// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

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


UENUM()
enum class ENCellJunctionFillDepthMode : uint8
{
	DefaultForward = 0,
	DefaultBackward = 1,
	DefaultCentered = 2,
	OverrideForward = 3,
	OverrideBackward = 4,
	OverrideCentered = 5,
};


/**
 * How a junction must be resolved during generation when it is left unconnected to another junction.
 *
 * Evaluated after the cell graph is linked: Required junctions force the generator to keep a connection,
 * while the Allow* variants permit an unconnected junction and choose what happens to the open socket.
 */
UENUM()
enum class ENCellJunctionRequirements : uint8
{
	Required =		0 UMETA(DisplayName = "Required", ToolTip = "This junction must have an associated junction socketed to it. Weights for required junctions are automatically doubled."),
	AllowBlocking = 1 UMETA(DisplayName = "Allow Blocking", ToolTip = "This junction will be filled if it is not linked to another junction."),
	AllowEmpty =	2 UMETA(DisplayName = "Allow Empty", ToolTip = "This junction will be left unfilled when not linked to another junction."),
};

/**
 * Persistent data describing a single junction on a cell.
 *
 * Stored both on UNCellJunctionComponent (live in the level) and in UNCell::Junctions (side-car asset)
 * so the two can be diffed and synced without loading the full level.
 */
USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNCellJunctionDetails
{
	GENERATED_BODY()

	/** Directionality of this junction. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	ENCellJunctionType Type = ENCellJunctionType::TwoWaySocket;

	/** Requirement satisfied during graph generation and spawning. */
	UPROPERTY(EditInstanceOnly)
	ENCellJunctionRequirements Requirements = ENCellJunctionRequirements::AllowBlocking;

	/** Size of the junction socket in grid units (width, height). */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	FIntVector2 SocketSize = FIntVector2(2, 4);

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	ENCellJunctionFillDepthMode FillDepthMode = ENCellJunctionFillDepthMode::DefaultForward;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta=(EditCondition="FillDepthMode != ENCellJunctionFillDepthMode::DefaultForward  && FillDepthMode != ENCellJunctionFillDepthMode::DefaultBackward && FillDepthMode != ENCellJunctionFillDepthMode::DefaultCentered",
		ClampMin="0.0", UIMin="0.0", Units="cm"))
	float OverrideFillDepth = 10.f;

	/** World-space location of the junction — derived from the component transform. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector WorldLocation = FVector::ZeroVector;

	/** World-space rotation of the junction — derived from the component transform. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRotator WorldRotation = FRotator::ZeroRotator;

	/** Allowed rotations for this junction when the owning cell is placed by the generator. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	FNRotationConstraints RotationConstraints;

	/**
	 * Relative weight for preferred selection.
	 * @note Higher values increase the probability of this junction being filled.
	 */
	UPROPERTY(EditInstanceOnly)
	int32 Weighting = 1;

	/** Unique per-cell identifier; allocated by ANCellActor::GetCellJunctionNextIdentifier. */
	UPROPERTY(VisibleAnywhere)
	int32 InstanceIdentifier = -1;

	/**
	 * Derived, non-serialized cache of WorldRotation.Quaternion().Inverse(), populated when the virtual cell data is
	 * assembled in FNVirtualOrganContext. Lets the per-candidate junction-rotation gate skip the rotator->quat
	 * conversion on every filter pass. Stays Identity on persistent (asset/component) instances; only meaningful on
	 * the virtual copies the generator filters against.
	 */
	FQuat CachedInverseWorldQuat = FQuat::Identity;

	/** Copy this struct into Other by value. @return Always true. */
	bool CopyTo(FNCellJunctionDetails& Other) const
	{
		Other = *this;
		return true;
	}

	/**
	 * Classifies a fill-depth mode into the fraction of the fill depth by which a filler's fill volume near edge is
	 * offset along the junction's forward axis before it extrudes forward by the depth. Shared by the junction component
	 * (runtime anchor) and the debug draw (fill-volume overlay) so the two never diverge.
	 * @param Mode The fill-depth mode to classify.
	 * @return 0 for the forward modes, -1 for the backward modes, and -0.5 for the centered modes.
	 */
	static float GetFillDepthAnchorScale(const ENCellJunctionFillDepthMode Mode)
	{
		switch (Mode)
		{
			using enum ENCellJunctionFillDepthMode;
		case DefaultBackward:
		case OverrideBackward:
			return -1.0f;
		case DefaultCentered:
		case OverrideCentered:
			return -0.5f;
		case DefaultForward:
		case OverrideForward:
		default:
			return 0.0f;
		}
	}

	/** @return true if every field matches structurally; used to detect author-time drift from the side-car. */
	bool IsEqual(const FNCellJunctionDetails& Other) const
	{
		return
			InstanceIdentifier == Other.InstanceIdentifier
			&& Weighting == Other.Weighting

			&& Requirements == Other.Requirements
			&& Type == Other.Type
			&& SocketSize == Other.SocketSize

			&& FillDepthMode == Other.FillDepthMode
			&& OverrideFillDepth == Other.OverrideFillDepth

			&& WorldLocation == Other.WorldLocation
			&& WorldRotation == Other.WorldRotation
			&& RotationConstraints.IsEqual(Other.RotationConstraints);
	}
};

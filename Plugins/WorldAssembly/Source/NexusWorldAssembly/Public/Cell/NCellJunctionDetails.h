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


/**
 * How deep a junction fill volume is, and which way it grows from the socket plane.
 *
 * The Default* modes take their depth from the project-wide Socket Depth; the Override* modes use the
 * junction's own Override Fill Depth instead. Forward grows along the junction facing, Backward opposite
 * it, and Centered straddles the plane. Fillers must honour this via
 * UNWorldAssemblyLibrary::GetJunctionFillDepthOffset -- one that ignores it always fills Forward.
 */
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
 * Per-junction override of the angle limits the connector pass gates candidate pairings on.
 *
 * The operation's limits (FNWorldAssemblyJunctionConnectorSettings) apply to every junction by default; one that
 * opts in here supplies its own instead, for any pairing it takes part in. Both ends of a pair are consulted and
 * the stricter limit wins, so an override can only ever narrow what a junction accepts — a permissive override on
 * one end never loosens a stricter partner.
 *
 * Only the routed connector pass reads these. Junctions the graph builder mated, and flush pairs picked up by
 * inverse matching, are aligned by construction and are not gated on angle at all.
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/types/cell-junction-connection-constraints/">FNCellJunctionConnectionConstraints</a>
 */
USTRUCT(BlueprintType)
struct FNCellJunctionConnectionConstraints
{
	GENERATED_BODY()

	/** Enables the limits below in place of the operation's. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, DisplayName="Override Angle Limits?",
		meta=(ToolTip="Should this junction gate its connector pairings on its own angle limits instead of the operation's?"))
	bool bOverrideAngleLimits = false;

	/** This junction's replacement for the operation's Maximum Facing Angle. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, DisplayName="Maximum Facing Angle",
		meta=(EditCondition="bOverrideAngleLimits", EditConditionHides, ClampMin="0", ClampMax="180", Units="deg",
			ToolTip="How far from directly facing each other this junction and its partner may be. 180 accepts any facing."))
	float MaximumFacingAngle = 90.f;

	/** This junction's replacement for the operation's Maximum Approach Angle. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, DisplayName="Maximum Approach Angle",
		meta=(EditCondition="bOverrideAngleLimits", EditConditionHides, ClampMin="0", ClampMax="180", Units="deg",
			ToolTip="How far off this junction's own facing its partner may sit. 180 accepts a partner anywhere, including directly behind."))
	float MaximumApproachAngle = 90.f;

	/** This junction's replacement for the operation's Maximum Elevation Difference. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, DisplayName="Maximum Elevation Difference",
		meta=(EditCondition="bOverrideAngleLimits", EditConditionHides, ClampMin="0", ClampMax="180", Units="deg",
			ToolTip="How far this junction and its partner may differ in how steeply they face up or down. 180 accepts any difference."))
	float MaximumElevationDifference = 45.f;

	/** @return true if every field matches; folded into FNCellJunctionDetails::IsEqual. */
	bool IsEqual(const FNCellJunctionConnectionConstraints& Other) const
	{
		return bOverrideAngleLimits == Other.bOverrideAngleLimits &&
			MaximumFacingAngle == Other.MaximumFacingAngle &&
			MaximumApproachAngle == Other.MaximumApproachAngle &&
			MaximumElevationDifference == Other.MaximumElevationDifference;
	}
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

	/**
	 * When true, the junction-connector pass will not pair this junction with another cell's.
	 *
	 * Only affects connectors. A junction opted out here still mates normally during graph building, and if it ends
	 * up unconnected it is still filled according to its Requirements.
	 * @note Lives here rather than beside UNCellJunctionComponent::bDisableFill because the connector pass runs
	 *       against the cell's side-car junction data on a worker thread and never sees the component. Filling
	 *       happens on the component itself at begin play, which is why that flag can live there.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, DisplayName="Disable Connecting",
		meta=(ToolTip="Should this junction be excluded from being paired up with another cell's junction by the connector pass?"))
	bool bDisableConnector = false;

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
	 * Optional per-junction tightening of the angle limits the connector pass pairs this junction on.
	 * @note Distinct from RotationConstraints above, which governs how the owning *cell* may be rotated when the
	 *       generator places it. By the time the connector pass runs both cells are already down and nothing is
	 *       being rotated, so what is gated there is the world-space relationship between two fixed openings.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, DisplayName="Connection Constraints")
	FNCellJunctionConnectionConstraints ConnectionConstraints;

	/**
	 * Relative weight for preferred selection.
	 * @note Higher values increase the probability of the graph builder picking this junction to extend from;
	 *       Required junctions have this doubled automatically (see FNOrganGraphBuilderTask).
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
			&& bDisableConnector == Other.bDisableConnector
			&& Type == Other.Type
			&& SocketSize == Other.SocketSize

			&& FillDepthMode == Other.FillDepthMode
			&& OverrideFillDepth == Other.OverrideFillDepth

			&& WorldLocation == Other.WorldLocation
			&& WorldRotation == Other.WorldRotation
			&& RotationConstraints.IsEqual(Other.RotationConstraints)
			&& ConnectionConstraints.IsEqual(Other.ConnectionConstraints);
	}
};

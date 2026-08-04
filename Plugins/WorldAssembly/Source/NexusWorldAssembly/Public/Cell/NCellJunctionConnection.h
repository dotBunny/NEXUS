// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellJunctionConnection.generated.h"

/**
 * A single routed curve, flattened to the polyline the connector pass actually validated.
 *
 * Stored sampled rather than as a curve definition because that is what was swept for collisions — a consumer that
 * re-derived the curve at a different resolution would not be looking at the geometry that was proven clear.
 */
USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNCellJunctionConnectorCurve
{
	GENERATED_BODY()

	/** World-space samples along the curve, start socket first. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FVector> Points;

	/** Summed length of the sampled polyline, in world units. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float Length = 0.f;
};

/**
 * The full cached route between two paired junctions: a center curve plus the four socket-corner curves that
 * bound the swept tube around it.
 *
 * The center curve is what the coarse radius sweep cleared; the corner curves are the exact volume the precise
 * pass cleared, and are what a connector actor should loft its geometry through. Corner ordering is consistent
 * end to end — Corners[k] starts at the start junction's corner k and ends at the end junction's matching corner —
 * so a consumer can build quads directly across the array without solving the correspondence itself.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/types/cell-junction-connection/">FNCellJunctionConnectorPath</a>
 */
USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNCellJunctionConnectorPath
{
	GENERATED_BODY()

	/** Control points of the center curve, in world space; the first and last sit at the two socket centers. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FVector> ControlPoints;

	/** Hermite tangent at each entry of ControlPoints, letting a consumer rebuild the exact curve on a USplineComponent. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FVector> ControlTangents;

	/** The center curve, sampled. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FNCellJunctionConnectorCurve Center;

	/** The four socket-corner curves, parallel in sample count to Center and in the start junction's corner order. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FNCellJunctionConnectorCurve> Corners;

	/** Spacing the curves were sampled at, so a consumer can reason about the resolution it is being handed. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float SampleStep = 0.f;

	/** @return true once the path holds a sampled center curve and its four corner curves. */
	bool IsValid() const
	{
		return Center.Points.Num() > 1 && Corners.Num() == 4;
	}
};

/**
 * One accepted junction pairing, recorded by the connector matching pass for the spawn stage to act on.
 *
 * The two ends are labelled Start and End by the deterministic ordering the matching pass fixes (lower graph index,
 * then node identifier). That ordering is what makes "the lowest cell node wins" well defined when resolving which
 * end's authored connector override takes priority, since node identifiers are only unique within a single graph.
 *
 * ConnectorIdentifier — not the node identifiers — is the key that rejoins the two ends at runtime: it is stamped
 * into both junctions' FNCellLinkDetails, which replicates with the cell, and pairs can span graphs.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/types/cell-junction-connection/">FNCellJunctionConnection</a>
 */
USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNCellJunctionConnection
{
	GENERATED_BODY()

	/** Operation-unique identifier for this pairing; carried on both ends' link details. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 ConnectorIdentifier = INDEX_NONE;

	/** Ticket of the operation that produced this pairing. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 OperationTicket = 0;

	/** Graph node identifier of the start cell. Only unique within that cell's own graph — see the struct docs. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 StartNodeIdentifier = INDEX_NONE;

	/** Instance identifier of the start junction, as carried on its FNCellJunctionDetails. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 StartJunctionInstanceIdentifier = INDEX_NONE;

	/** Identifier of the organ whose graph placed the start cell, used to resolve that organ's connector overrides. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FGuid StartOrganIdentifier;

	/** World-space frame of the start socket the path was solved against. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FTransform StartTransform = FTransform::Identity;

	/** Graph node identifier of the end cell. Only unique within that cell's own graph — see the struct docs. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 EndNodeIdentifier = INDEX_NONE;

	/** Instance identifier of the end junction, as carried on its FNCellJunctionDetails. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 EndJunctionInstanceIdentifier = INDEX_NONE;

	/** Identifier of the organ whose graph placed the end cell, used to resolve that organ's connector overrides. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FGuid EndOrganIdentifier;

	/** World-space frame of the end socket the path was solved against. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FTransform EndTransform = FTransform::Identity;

	/** The cleared route between the two sockets. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FNCellJunctionConnectorPath Path;

	FString ToString() const
	{
		return FString::Printf(TEXT("[Connector %d] [%d:%d] <-> [%d:%d] | Length: %.1f"),
			ConnectorIdentifier,
			StartNodeIdentifier, StartJunctionInstanceIdentifier,
			EndNodeIdentifier, EndJunctionInstanceIdentifier,
			Path.Center.Length);
	}
};

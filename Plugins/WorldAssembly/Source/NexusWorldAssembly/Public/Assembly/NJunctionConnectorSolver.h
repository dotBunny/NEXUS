// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NWorldAssemblySettings.h"
#include "Cell/NCellJunctionConnection.h"
#include "Cell/NCellJunctionDetails.h"

struct FNRawMesh;

/** One cross-section frame along a routed connector path: where it is, which way it heads, and how it is oriented. */
struct FNJunctionConnectorFrame
{
	/** World-space position of this sample on the center curve. */
	FVector Location = FVector::ZeroVector;

	/** Unit direction of travel, start socket toward end socket. */
	FVector Tangent = FVector::ForwardVector;

	/** Unit socket-width axis, transported from the start junction's own right axis. */
	FVector Right = FVector::RightVector;

	/** Unit socket-height axis, derived as Right x Tangent so it stays orthogonal without a second transport. */
	FVector Up = FVector::UpVector;
};

/**
 * One candidate route between two junctions: the curve that defines it, the frames sampled along it, and the
 * path record handed downstream if it survives collision testing.
 */
struct FNJunctionConnectorRoute
{
	/** Frames at each sample of the center curve; parallel to Path.Center.Points. */
	TArray<FNJunctionConnectorFrame> Frames;

	/** The path record, populated with the center curve, the four corner curves, and the control points. */
	FNCellJunctionConnectorPath Path;
};

/**
 * Geometry for routing a connector between two unmatched junctions: curve construction, sampling, socket-corner
 * correspondence, and the swept hulls used to prove the route is clear.
 *
 * Deliberately free of UObjects, worlds, and randomness — the connector pass runs on a worker thread, and every
 * decision here has to be reproducible for a given input so two runs of the same seed route identically. Collision
 * testing is not part of this class; it produces the hulls and the caller decides what they intersect.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/types/junction-connector-solver/">FNJunctionConnectorSolver</a>
 */
class NEXUSWORLDASSEMBLY_API FNJunctionConnectorSolver
{
public:
	/** Hard ceiling on samples per curve, so a pathological length/step ratio cannot blow up the sweep cost. */
	static constexpr int32 MaximumSampleCount = 512;

	/** Number of perpendicular directions each avoidance ring pushes a detour midpoint along. */
	static constexpr int32 AvoidanceDirectionCount = 8;

	/**
	 * Evaluate a piecewise cubic Hermite curve.
	 * @param ControlPoints Curve control points; at least two.
	 * @param Tangents Tangent at each control point, parallel to ControlPoints.
	 * @param Alpha Position along the whole curve in the 0..1 range; clamped.
	 * @return The world-space point at Alpha, or ZeroVector when the inputs are malformed.
	 */
	static FVector EvaluateCurve(const TArray<FVector>& ControlPoints, const TArray<FVector>& Tangents, float Alpha);

	/**
	 * Build the control points and tangents for a route between two junctions.
	 *
	 * The curve leaves the start socket along the direction that junction opens onto and arrives at the end socket
	 * travelling into the cell that owns it, so a connector welded to both ends meets each socket square on.
	 * @param Start The junction the route leaves from.
	 * @param End The junction the route arrives at.
	 * @param TangentScale Tangent magnitude as a fraction of the straight-line distance between the two sockets.
	 * @param MidPoint Optional detour point the curve is routed through; null builds the direct two-point curve.
	 * @param OutControlPoints Receives the control points. Emptied first.
	 * @param OutTangents Receives the matching tangents. Emptied first.
	 */
	static void BuildControlPoints(const FNCellJunctionDetails& Start, const FNCellJunctionDetails& End,
		float TangentScale, const FVector* MidPoint, TArray<FVector>& OutControlPoints, TArray<FVector>& OutTangents);

	/**
	 * Flatten a curve to a polyline at roughly SampleStep spacing.
	 *
	 * Resolution is chosen from a coarse length estimate rather than from the control polygon, so a curve that bows
	 * well outside its control points is not under-sampled.
	 * @param ControlPoints Curve control points.
	 * @param Tangents Tangent at each control point.
	 * @param SampleStep Target spacing between samples.
	 * @param OutPoints Receives the samples, first and last landing exactly on the terminal control points. Emptied first.
	 * @return Summed length of the emitted polyline.
	 */
	static float SampleCurve(const TArray<FVector>& ControlPoints, const TArray<FVector>& Tangents,
		float SampleStep, TArray<FVector>& OutPoints);

	/**
	 * Choose which corner of the end socket each corner of the start socket connects to.
	 *
	 * Only the four mappings that preserve the rectangle's edges are considered — a tube joining two rectangles
	 * cannot pair corners arbitrarily without folding — and on a non-square socket the two that would map a width
	 * edge onto a height edge are dropped as well. The survivor with the least total corner travel wins, so the
	 * result is the least-twisted join available.
	 * @param StartCorners The start socket's four world-space corners.
	 * @param EndCorners The end socket's four world-space corners, in its own corner order.
	 * @param bSquareSocket True when the socket's width and height match, which admits the two quarter-turn mappings.
	 * @return For each start corner index, the index of the end corner it pairs with.
	 */
	static TStaticArray<int32, 4> ResolveCornerPairing(const TArray<FVector>& StartCorners, const TArray<FVector>& EndCorners,
		bool bSquareSocket);

	/**
	 * Build a complete candidate route, including the four corner curves.
	 * @param Start The junction the route leaves from.
	 * @param End The junction the route arrives at.
	 * @param SocketUnitSize World size of a single socket grid unit, normally UNWorldAssemblySettings::SocketSize.
	 * @param Settings Connector tuning supplying the tangent scale, sample step, and length limit.
	 * @param MidPoint Optional detour point the curve is routed through; null builds the direct route.
	 * @param OutRoute Receives the built route. Left in an unspecified state when the call returns false.
	 * @return false when the center curve or any corner curve exceeds the configured maximum spline length, or when
	 *         the two sockets are too close together to define a curve at all.
	 * @note Length is checked before the corner curves are built where possible, so a route that is obviously too
	 *       long costs little.
	 */
	static bool BuildRoute(const FNCellJunctionDetails& Start, const FNCellJunctionDetails& End,
		const FVector2D& SocketUnitSize, const FNWorldAssemblyJunctionConnectorSettings& Settings,
		const FVector* MidPoint, FNJunctionConnectorRoute& OutRoute);

	/**
	 * Deterministic detour midpoints to try when the direct route is blocked, ordered least-deviating first.
	 *
	 * Candidates fan out in rings perpendicular to the straight line between the sockets, so the first accepted
	 * detour is the smallest one that clears. No randomness is involved: the same pair always produces the same
	 * sequence.
	 * @param Start World-space start socket center.
	 * @param End World-space end socket center.
	 * @param OffsetStep Distance each successive ring adds.
	 * @param MaximumAttempts Upper bound on emitted candidates.
	 * @param OutMidPoints Receives the candidates. Emptied first.
	 */
	static void BuildAvoidanceMidPoints(const FVector& Start, const FVector& End,
		float OffsetStep, int32 MaximumAttempts, TArray<FVector>& OutMidPoints);

	/**
	 * Build one convex prism per path segment, with a square cross-section of half-extent Radius.
	 *
	 * The coarse clearance volume: cheaper to test than the socket-shaped tube and independent of socket size, so a
	 * route that is plainly buried in geometry is rejected before the exact hulls are built.
	 * @param Route The route to sweep.
	 * @param Radius Half-extent of the square cross-section.
	 * @param OutHulls Receives one prism per segment. Emptied first.
	 */
	static void BuildRadiusHulls(const FNJunctionConnectorRoute& Route, float Radius, TArray<FNRawMesh>& OutHulls);

	/**
	 * Build one convex prism per path segment spanning the four corner curves — the exact volume a connector's
	 * geometry occupies.
	 * @param Route The route to sweep; must carry four corner curves.
	 * @param OutHulls Receives one prism per segment. Emptied first.
	 */
	static void BuildCornerHulls(const FNJunctionConnectorRoute& Route, TArray<FNRawMesh>& OutHulls);

private:
	/**
	 * Transport an orientation along the sampled center curve and correct it to land on the end socket.
	 *
	 * Uses the double-reflection rotation-minimizing frame, which introduces no roll of its own, then measures
	 * whatever roll remains against the required end orientation and spreads it evenly along the curve. The
	 * alternative — interpolating the two socket orientations directly — twists the tube wherever the curve bends.
	 * @param Points Sampled center curve.
	 * @param StartTangent Unit travel direction leaving the start socket.
	 * @param EndTangent Unit travel direction arriving at the end socket.
	 * @param StartRight The start socket's width axis, which seeds the transport.
	 * @param TargetEndRight The width axis the transported frame must arrive at, under the chosen corner pairing.
	 * @param OutFrames Receives one frame per sample. Emptied first.
	 */
	static void BuildFrames(const TArray<FVector>& Points, const FVector& StartTangent, const FVector& EndTangent,
		const FVector& StartRight, const FVector& TargetEndRight, TArray<FNJunctionConnectorFrame>& OutFrames);

	/**
	 * Recover the width axis implied by an end socket's corners once the pairing has permuted them.
	 * @param PairedEndCorners The end socket's corners, reordered so entry k pairs with start corner k.
	 * @param EndCenter World-space center of the end socket.
	 * @return The unit width axis, or the zero vector for a degenerate socket.
	 */
	static FVector ResolveEndRightAxis(const TStaticArray<FVector, 4>& PairedEndCorners, const FVector& EndCenter);
};

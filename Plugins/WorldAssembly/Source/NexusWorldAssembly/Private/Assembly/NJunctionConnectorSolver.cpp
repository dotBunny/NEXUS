// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/NJunctionConnectorSolver.h"

#include "NWorldAssemblyUtils.h"
#include "Types/NRawMeshUtils.h"

/** Samples taken when estimating a curve's length before choosing its real sample count. */
static constexpr int32 LengthEstimateSampleCount = 32;

/** Squared-length floor below which a direction is treated as degenerate rather than normalized. */
static constexpr double DegenerateLengthSquared = UE_DOUBLE_SMALL_NUMBER;

/**
 * Sign pattern of each socket corner along the width axis, in the corner order
 * FNWorldAssemblyUtils::GetJunctionWorldCornerPoints emits (top-left, bottom-left, bottom-right, top-right).
 */
static constexpr double CornerWidthSign[4] = { -1.0, -1.0, 1.0, 1.0 };

/** Sign pattern of each socket corner along the height axis, in the same corner order. */
static constexpr double CornerHeightSign[4] = { 1.0, -1.0, -1.0, 1.0 };

/** @return Vector normalized, or Fallback when it is too short to have a meaningful direction. */
static FVector SafeDirection(const FVector& Vector, const FVector& Fallback)
{
	const double LengthSquared = Vector.SizeSquared();
	if (LengthSquared <= DegenerateLengthSquared)
	{
		return Fallback;
	}
	return Vector * (1.0 / FMath::Sqrt(LengthSquared));
}

FVector FNJunctionConnectorSolver::EvaluateCurve(const TArray<FVector>& ControlPoints, const TArray<FVector>& Tangents, const float Alpha)
{
	const int32 PointCount = ControlPoints.Num();
	if (PointCount < 2 || Tangents.Num() != PointCount)
	{
		return FVector::ZeroVector;
	}

	const int32 SegmentCount = PointCount - 1;
	const double Scaled = FMath::Clamp(static_cast<double>(Alpha), 0.0, 1.0) * SegmentCount;

	// Clamped so Alpha == 1 lands on the final segment's t == 1 rather than indexing one segment past the end.
	const int32 Segment = FMath::Clamp(FMath::FloorToInt32(Scaled), 0, SegmentCount - 1);
	const double t = Scaled - Segment;
	const double t2 = t * t;
	const double t3 = t2 * t;

	// Cubic Hermite basis.
	const double H00 = 2.0 * t3 - 3.0 * t2 + 1.0;
	const double H10 = t3 - 2.0 * t2 + t;
	const double H01 = -2.0 * t3 + 3.0 * t2;
	const double H11 = t3 - t2;

	return ControlPoints[Segment] * H00 + Tangents[Segment] * H10
		+ ControlPoints[Segment + 1] * H01 + Tangents[Segment + 1] * H11;
}

void FNJunctionConnectorSolver::BuildControlPoints(const FNCellJunctionDetails& Start, const FNCellJunctionDetails& End,
	const float TangentScale, const FVector* MidPoint, TArray<FVector>& OutControlPoints, TArray<FVector>& OutTangents)
{
	OutControlPoints.Reset();
	OutTangents.Reset();

	const FVector StartLocation = Start.WorldLocation;
	const FVector EndLocation = End.WorldLocation;
	const double Distance = FVector::Dist(StartLocation, EndLocation);
	const double TangentLength = Distance * TangentScale;

	// The route leaves the start socket along the direction it opens onto, and arrives at the end socket travelling
	// into the cell that owns it — which is the opposite of the direction that socket opens onto.
	const FVector StartTangent = FNWorldAssemblyUtils::GetJunctionOutwardDirection(Start) * TangentLength;
	const FVector EndTangent = -FNWorldAssemblyUtils::GetJunctionOutwardDirection(End) * TangentLength;

	OutControlPoints.Add(StartLocation);
	OutTangents.Add(StartTangent);

	if (MidPoint != nullptr)
	{
		// Catmull-Rom style: the detour point's tangent follows the chord across it, so the curve passes through
		// without the kink an independently chosen direction would introduce.
		const FVector MidTangent = SafeDirection(EndLocation - StartLocation, FVector::ForwardVector) * TangentLength;
		OutControlPoints.Add(*MidPoint);
		OutTangents.Add(MidTangent);
	}

	OutControlPoints.Add(EndLocation);
	OutTangents.Add(EndTangent);
}

float FNJunctionConnectorSolver::SampleCurve(const TArray<FVector>& ControlPoints, const TArray<FVector>& Tangents,
	const float SampleStep, TArray<FVector>& OutPoints)
{
	OutPoints.Reset();

	const int32 PointCount = ControlPoints.Num();
	if (PointCount < 2 || Tangents.Num() != PointCount)
	{
		return 0.f;
	}

	// Estimate the length first: a curve with strong tangents can bow well outside its control polygon, so sizing
	// the sample count off the control points alone would under-sample exactly the routes that bend the most.
	double EstimatedLength = 0.0;
	FVector Previous = EvaluateCurve(ControlPoints, Tangents, 0.f);
	for (int32 i = 1; i <= LengthEstimateSampleCount; i++)
	{
		const FVector Current = EvaluateCurve(ControlPoints, Tangents, static_cast<float>(i) / LengthEstimateSampleCount);
		EstimatedLength += FVector::Dist(Previous, Current);
		Previous = Current;
	}

	const int32 SegmentCount = FMath::Clamp(
		FMath::CeilToInt32(EstimatedLength / FMath::Max(SampleStep, 1.f)), 1, MaximumSampleCount - 1);

	OutPoints.Reserve(SegmentCount + 1);
	double Length = 0.0;
	for (int32 i = 0; i <= SegmentCount; i++)
	{
		const FVector Current = EvaluateCurve(ControlPoints, Tangents, static_cast<float>(i) / SegmentCount);
		if (i > 0)
		{
			Length += FVector::Dist(OutPoints.Last(), Current);
		}
		OutPoints.Add(Current);
	}

	return static_cast<float>(Length);
}

TStaticArray<int32, 4> FNJunctionConnectorSolver::ResolveCornerPairing(const TArray<FVector>& StartCorners,
	const TArray<FVector>& EndCorners, const bool bSquareSocket)
{
	TStaticArray<int32, 4> Pairing;
	for (int32 i = 0; i < 4; i++)
	{
		Pairing[i] = i;
	}

	if (StartCorners.Num() != 4 || EndCorners.Num() != 4)
	{
		return Pairing;
	}

	// Both sockets wind their corners the same way, but the tube sees the end socket from behind, so a join that
	// does not fold has to reverse that winding: Map(i) = (Shift - i) mod 4. Odd shifts map width edges onto width
	// edges and height onto height; even shifts swap the two, which only fits a square socket.
	int32 BestShift = INDEX_NONE;
	double BestCost = 0.0;
	for (int32 Shift = 0; Shift < 4; Shift++)
	{
		if (!bSquareSocket && (Shift % 2) == 0)
		{
			continue;
		}

		double Cost = 0.0;
		for (int32 i = 0; i < 4; i++)
		{
			Cost += FVector::DistSquared(StartCorners[i], EndCorners[((Shift - i) % 4 + 4) % 4]);
		}

		// Strictly-less keeps the lowest shift on a tie, so an ambiguous square socket still resolves identically
		// every run.
		if (BestShift == INDEX_NONE || Cost < BestCost)
		{
			BestShift = Shift;
			BestCost = Cost;
		}
	}

	for (int32 i = 0; i < 4; i++)
	{
		Pairing[i] = ((BestShift - i) % 4 + 4) % 4;
	}
	return Pairing;
}

FVector FNJunctionConnectorSolver::ResolveEndRightAxis(const TStaticArray<FVector, 4>& PairedEndCorners, const FVector& EndCenter)
{
	// Averages the two corners on the positive width side against the two on the negative side. Reading the axis off
	// a single corner pair would work for an exact rectangle but skews on the slightly-off inputs a rotated socket
	// can produce; this stays centered.
	FVector Axis = FVector::ZeroVector;
	for (int32 i = 0; i < 4; i++)
	{
		Axis += (PairedEndCorners[i] - EndCenter) * CornerWidthSign[i];
	}
	return SafeDirection(Axis, FVector::ZeroVector);
}

void FNJunctionConnectorSolver::BuildFrames(const TArray<FVector>& Points, const FVector& StartTangent, const FVector& EndTangent,
	const FVector& StartRight, const FVector& TargetEndRight, TArray<FNJunctionConnectorFrame>& OutFrames)
{
	OutFrames.Reset();

	const int32 PointCount = Points.Num();
	if (PointCount < 2)
	{
		return;
	}

	OutFrames.SetNum(PointCount);

	// Tangents: the terminals are pinned to the socket normals so the tube meets each opening square on, and the
	// interior uses central differences, which track the curve better than a one-sided difference at the same cost.
	for (int32 i = 0; i < PointCount; i++)
	{
		FNJunctionConnectorFrame& Frame = OutFrames[i];
		Frame.Location = Points[i];

		if (i == 0)
		{
			Frame.Tangent = StartTangent;
		}
		else if (i == PointCount - 1)
		{
			Frame.Tangent = EndTangent;
		}
		else
		{
			Frame.Tangent = SafeDirection(Points[i + 1] - Points[i - 1], StartTangent);
		}
	}

	// Double-reflection rotation-minimizing frame (Wang et al.): reflect the reference axis across the segment, then
	// across the plane that aligns the reflected tangent with the next one. Introduces no roll of its own, which is
	// what stops the tube corkscrewing around a bend.
	OutFrames[0].Right = StartRight;
	for (int32 i = 0; i < PointCount - 1; i++)
	{
		const FVector& CurrentRight = OutFrames[i].Right;
		const FVector& CurrentTangent = OutFrames[i].Tangent;
		const FVector& NextTangent = OutFrames[i + 1].Tangent;

		const FVector SegmentVector = Points[i + 1] - Points[i];
		const double SegmentLengthSquared = SegmentVector.SizeSquared();
		if (SegmentLengthSquared <= DegenerateLengthSquared)
		{
			// A zero-length segment has no reflection plane; carrying the axis through unchanged is the limit case.
			OutFrames[i + 1].Right = CurrentRight;
			continue;
		}

		const double SegmentScale = 2.0 / SegmentLengthSquared;
		const FVector ReflectedRight = CurrentRight - SegmentVector * (SegmentScale * FVector::DotProduct(SegmentVector, CurrentRight));
		const FVector ReflectedTangent = CurrentTangent - SegmentVector * (SegmentScale * FVector::DotProduct(SegmentVector, CurrentTangent));

		const FVector TangentDelta = NextTangent - ReflectedTangent;
		const double DeltaLengthSquared = TangentDelta.SizeSquared();
		if (DeltaLengthSquared <= DegenerateLengthSquared)
		{
			// The reflected tangent already matches; the second reflection would be the identity.
			OutFrames[i + 1].Right = ReflectedRight;
			continue;
		}

		OutFrames[i + 1].Right = ReflectedRight
			- TangentDelta * ((2.0 / DeltaLengthSquared) * FVector::DotProduct(TangentDelta, ReflectedRight));
	}

	// The transport lands wherever it lands; the tube still has to weld to the end socket's actual corners. Measure
	// the leftover roll about the arrival tangent and spread it evenly, which keeps both ends exact and puts the
	// unavoidable twist where it is least visible rather than all at one opening.
	double RollAngle = 0.0;
	if (!TargetEndRight.IsNearlyZero())
	{
		const FVector& FinalTangent = OutFrames.Last().Tangent;

		// Project both axes into the plane the roll happens in, so a small tangent mismatch cannot leak into the angle.
		const FVector TransportedFlat = SafeDirection(
			FVector::VectorPlaneProject(OutFrames.Last().Right, FinalTangent), FVector::ZeroVector);
		const FVector TargetFlat = SafeDirection(
			FVector::VectorPlaneProject(TargetEndRight, FinalTangent), FVector::ZeroVector);

		if (!TransportedFlat.IsNearlyZero() && !TargetFlat.IsNearlyZero())
		{
			const double CosAngle = FMath::Clamp(FVector::DotProduct(TransportedFlat, TargetFlat), -1.0, 1.0);
			const double SinAngle = FVector::DotProduct(FVector::CrossProduct(TransportedFlat, TargetFlat), FinalTangent);
			RollAngle = FMath::Atan2(SinAngle, CosAngle);
		}
	}

	const int32 LastIndex = PointCount - 1;
	for (int32 i = 0; i < PointCount; i++)
	{
		FNJunctionConnectorFrame& Frame = OutFrames[i];
		if (!FMath::IsNearlyZero(RollAngle) && i > 0)
		{
			const double Alpha = static_cast<double>(i) / LastIndex;
			Frame.Right = Frame.Right.RotateAngleAxisRad(RollAngle * Alpha, Frame.Tangent);
		}

		// Re-orthogonalize against the tangent: the reflections preserve the angle in exact arithmetic, but a long
		// chain of them drifts, and a skewed frame would shear the swept prisms.
		Frame.Right = SafeDirection(FVector::VectorPlaneProject(Frame.Right, Frame.Tangent), FVector::RightVector);
		Frame.Up = FVector::CrossProduct(Frame.Right, Frame.Tangent);
	}
}

float FNJunctionConnectorSolver::GetMinimumTurnRadiusScale(const FNJunctionConnectorRoute& Route, const FVector2D& SocketWorldSize)
{
	const TArray<FVector>& Points = Route.Path.Center.Points;
	const int32 PointCount = Points.Num();
	if (PointCount < 3 || Route.Frames.Num() != PointCount)
	{
		return MAX_flt;
	}

	const double HalfWidth = SocketWorldSize.X * 0.5;
	const double HalfHeight = SocketWorldSize.Y * 0.5;

	double MinimumScale = MAX_flt;
	for (int32 i = 1; i < PointCount - 1; i++)
	{
		const FVector& Previous = Points[i - 1];
		const FVector& Current = Points[i];
		const FVector& Next = Points[i + 1];

		const FVector Incoming = Current - Previous;
		const FVector Outgoing = Next - Current;

		// Menger curvature: the radius of the circle through three consecutive samples. Robust on the unevenly
		// spaced samples the parameter-uniform sampler produces, where a difference-based estimate would skew with
		// the spacing.
		const double TwiceArea = FVector::CrossProduct(Incoming, Outgoing).Size();
		if (TwiceArea <= DegenerateLengthSquared)
		{
			// Collinear samples: no turn here to measure.
			continue;
		}

		const double Radius = (Incoming.Size() * Outgoing.Size() * (Next - Previous).Size()) / (2.0 * TwiceArea);

		// The direction the curve is bending toward. The second difference points at the center of curvature, and
		// projecting out the tangent leaves the pure turn direction — in whatever plane the turn happens to be in,
		// which is what keeps this honest for vertical and oblique turns rather than just yaw.
		const FNJunctionConnectorFrame& Frame = Route.Frames[i];
		const FVector TurnNormal = SafeDirection(
			FVector::VectorPlaneProject(Incoming.GetSafeNormal() - Outgoing.GetSafeNormal(), Frame.Tangent),
			FVector::ZeroVector);
		if (TurnNormal.IsNearlyZero())
		{
			continue;
		}

		// How far the connector's own geometry reaches toward the inside of this turn. A turn toward the socket's
		// up axis has to clear its half-height; one toward the right axis, its half-width; anything between, a
		// blend. This is what makes a single ratio meaningful for every turn plane and socket shape.
		const double Extent = FMath::Abs(FVector::DotProduct(Frame.Right, TurnNormal)) * HalfWidth
			+ FMath::Abs(FVector::DotProduct(Frame.Up, TurnNormal)) * HalfHeight;
		if (Extent <= UE_DOUBLE_SMALL_NUMBER)
		{
			continue;
		}

		MinimumScale = FMath::Min(MinimumScale, Radius / Extent);
	}

	return static_cast<float>(MinimumScale);
}

bool FNJunctionConnectorSolver::DoesRouteFold(const FNJunctionConnectorRoute& Route)
{
	if (Route.Path.Corners.Num() != 4)
	{
		return false;
	}

	const int32 FrameCount = Route.Frames.Num();
	for (const FNCellJunctionConnectorCurve& Corner : Route.Path.Corners)
	{
		const int32 PointCount = Corner.Points.Num();
		if (PointCount != FrameCount)
		{
			continue;
		}

		for (int32 i = 1; i < PointCount; i++)
		{
			// Once the turn is tighter than the socket reaches, the inside corner of the connector travels backwards
			// while the center still moves forward — the wall passes through itself. Comparing the corner's own step
			// against the direction of travel catches exactly that, on the points a connector would be handed.
			if (FVector::DotProduct(Corner.Points[i] - Corner.Points[i - 1], Route.Frames[i - 1].Tangent) <= 0.0)
			{
				return true;
			}
		}
	}

	return false;
}

ENJunctionConnectorRouteResult FNJunctionConnectorSolver::BuildRoute(const FNCellJunctionDetails& Start, const FNCellJunctionDetails& End,
	const FVector2D& SocketUnitSize, const FNWorldAssemblyJunctionConnectorSettings& Settings,
	const FVector* MidPoint, const float TangentScale, FNJunctionConnectorRoute& OutRoute)
{
	OutRoute.Frames.Reset();
	OutRoute.Path = FNCellJunctionConnectorPath();
	OutRoute.Path.SampleStep = Settings.SampleStep;

	// Coincident sockets have no direction to route along, and nothing sensible to hand a connector actor.
	if (FVector::DistSquared(Start.WorldLocation, End.WorldLocation) <= DegenerateLengthSquared)
	{
		return ENJunctionConnectorRouteResult::Degenerate;
	}

	BuildControlPoints(Start, End, TangentScale, MidPoint, OutRoute.Path.ControlPoints, OutRoute.Path.ControlTangents);

	OutRoute.Path.Center.Length = SampleCurve(OutRoute.Path.ControlPoints, OutRoute.Path.ControlTangents,
		Settings.SampleStep, OutRoute.Path.Center.Points);

	if (OutRoute.Path.Center.Points.Num() < 2)
	{
		return ENJunctionConnectorRouteResult::Degenerate;
	}

	// Checked before any corner work: the corners can only be longer than the center on the outside of a bend, so a
	// center that already blows the budget can never be rescued.
	if (OutRoute.Path.Center.Length > Settings.MaximumSplineLength)
	{
		return ENJunctionConnectorRouteResult::TooLong;
	}

	const TArray<FVector> StartCorners = FNWorldAssemblyUtils::GetJunctionWorldCornerPoints(Start, SocketUnitSize);
	const TArray<FVector> EndCorners = FNWorldAssemblyUtils::GetJunctionWorldCornerPoints(End, SocketUnitSize);
	if (StartCorners.Num() != 4 || EndCorners.Num() != 4)
	{
		return ENJunctionConnectorRouteResult::Degenerate;
	}

	const FVector2D SocketWorldSize = FNWorldAssemblyUtils::GetWorldSize2D(Start.SocketSize, SocketUnitSize);
	const bool bSquareSocket = FMath::IsNearlyEqual(SocketWorldSize.X, SocketWorldSize.Y);
	const TStaticArray<int32, 4> Pairing = ResolveCornerPairing(StartCorners, EndCorners, bSquareSocket);

	TStaticArray<FVector, 4> PairedEndCorners;
	for (int32 i = 0; i < 4; i++)
	{
		PairedEndCorners[i] = EndCorners[Pairing[i]];
	}

	const FVector StartTangent = FNWorldAssemblyUtils::GetJunctionOutwardDirection(Start);
	const FVector EndTangent = -FNWorldAssemblyUtils::GetJunctionOutwardDirection(End);
	const FQuat StartQuat = Start.WorldRotation.Quaternion();

	BuildFrames(OutRoute.Path.Center.Points, StartTangent, EndTangent,
		StartQuat.GetAxisY(), ResolveEndRightAxis(PairedEndCorners, End.WorldLocation), OutRoute.Frames);

	// Offsets are read off the start socket rather than recomputed from the socket size, so a corner curve begins on
	// the exact vertex the cell authored even when the two differ by a rounding step.
	const double HalfWidth = SocketWorldSize.X * 0.5;
	const double HalfHeight = SocketWorldSize.Y * 0.5;

	const int32 SampleCount = OutRoute.Path.Center.Points.Num();
	OutRoute.Path.Corners.SetNum(4);
	for (int32 Corner = 0; Corner < 4; Corner++)
	{
		FNCellJunctionConnectorCurve& CornerCurve = OutRoute.Path.Corners[Corner];
		CornerCurve.Points.Reset(SampleCount);

		const double WidthOffset = CornerWidthSign[Corner] * HalfWidth;
		const double HeightOffset = CornerHeightSign[Corner] * HalfHeight;

		double CornerLength = 0.0;
		for (int32 i = 0; i < SampleCount; i++)
		{
			const FNJunctionConnectorFrame& Frame = OutRoute.Frames[i];
			const FVector Point = Frame.Location + Frame.Right * WidthOffset + Frame.Up * HeightOffset;
			if (i > 0)
			{
				CornerLength += FVector::Dist(CornerCurve.Points.Last(), Point);
			}
			CornerCurve.Points.Add(Point);
		}

		// Pin the terminals to the authored socket vertices: the frames land on them to within floating point, and a
		// connector welding geometry to a socket cannot tolerate even that much gap.
		CornerCurve.Points[0] = StartCorners[Corner];
		CornerCurve.Points.Last() = PairedEndCorners[Corner];
		CornerCurve.Length = static_cast<float>(CornerLength);

		if (CornerCurve.Length > Settings.MaximumSplineLength)
		{
			return ENJunctionConnectorRouteResult::TooLong;
		}
	}

	// Always rejected, whatever the configured minimum: a folded route is one whose geometry passes through itself,
	// which no connector can build. Tested first because it is exact and cheap, where the radius below is a measure.
	if (DoesRouteFold(OutRoute))
	{
		return ENJunctionConnectorRouteResult::TooTight;
	}

	// Zero opts out of the navigability floor, leaving only the fold rejection above.
	if (Settings.MinimumTurnRadiusScale > 0.f
		&& GetMinimumTurnRadiusScale(OutRoute, SocketWorldSize) < Settings.MinimumTurnRadiusScale)
	{
		return ENJunctionConnectorRouteResult::TooTight;
	}

	return ENJunctionConnectorRouteResult::Success;
}

void FNJunctionConnectorSolver::BuildAvoidanceMidPoints(const FVector& Start, const FVector& End,
	const float OffsetStep, const int32 MaximumAttempts, TArray<FVector>& OutMidPoints)
{
	OutMidPoints.Reset();
	if (MaximumAttempts <= 0)
	{
		return;
	}

	const FVector ChordDirection = SafeDirection(End - Start, FVector::ForwardVector);

	// Any perpendicular basis works as long as it is derived the same way every run. Up is the natural first choice
	// so the ring reads as left/right/over/under; the fallback only matters for a perfectly vertical chord.
	FVector Right = FVector::CrossProduct(ChordDirection, FVector::UpVector);
	if (Right.SizeSquared() <= DegenerateLengthSquared)
	{
		Right = FVector::CrossProduct(ChordDirection, FVector::ForwardVector);
	}
	Right = SafeDirection(Right, FVector::RightVector);
	const FVector Up = FVector::CrossProduct(Right, ChordDirection).GetSafeNormal();

	const double Diagonal = UE_DOUBLE_INV_SQRT_2;
	const TStaticArray<FVector, AvoidanceDirectionCount> Directions{
		Right,
		Up,
		-Right,
		-Up,
		(Right + Up) * Diagonal,
		(-Right + Up) * Diagonal,
		(-Right - Up) * Diagonal,
		(Right - Up) * Diagonal,
	};

	const FVector ChordMidPoint = (Start + End) * 0.5;
	const int32 RingCount = FMath::DivideAndRoundUp(MaximumAttempts, AvoidanceDirectionCount);

	OutMidPoints.Reserve(MaximumAttempts);
	// Ring-major so the smallest detour that clears is the one that wins, rather than whichever direction happens to
	// be tried first at an arbitrary distance.
	for (int32 Ring = 1; Ring <= RingCount; Ring++)
	{
		const double Distance = Ring * OffsetStep;
		for (int32 Direction = 0; Direction < AvoidanceDirectionCount; Direction++)
		{
			if (OutMidPoints.Num() >= MaximumAttempts)
			{
				return;
			}
			OutMidPoints.Add(ChordMidPoint + Directions[Direction] * Distance);
		}
	}
}

void FNJunctionConnectorSolver::BuildRadiusHulls(const FNJunctionConnectorRoute& Route, const float Radius, TArray<FNRawMesh>& OutHulls)
{
	OutHulls.Reset();

	const int32 FrameCount = Route.Frames.Num();
	if (FrameCount < 2)
	{
		return;
	}

	auto CornersAt = [Radius](const FNJunctionConnectorFrame& Frame)
	{
		TStaticArray<FVector, 4> Corners;
		for (int32 i = 0; i < 4; i++)
		{
			Corners[i] = Frame.Location + Frame.Right * (CornerWidthSign[i] * Radius) + Frame.Up * (CornerHeightSign[i] * Radius);
		}
		return Corners;
	};

	OutHulls.Reserve(FrameCount - 1);
	TStaticArray<FVector, 4> NearCorners = CornersAt(Route.Frames[0]);
	for (int32 i = 1; i < FrameCount; i++)
	{
		TStaticArray<FVector, 4> FarCorners = CornersAt(Route.Frames[i]);
		OutHulls.Add(FNRawMeshUtils::MakeConvexPrism(NearCorners, FarCorners));
		NearCorners = FarCorners;
	}
}

void FNJunctionConnectorSolver::BuildCornerHulls(const FNJunctionConnectorRoute& Route, TArray<FNRawMesh>& OutHulls)
{
	OutHulls.Reset();

	if (Route.Path.Corners.Num() != 4)
	{
		return;
	}

	const int32 SampleCount = Route.Path.Corners[0].Points.Num();
	if (SampleCount < 2)
	{
		return;
	}

	OutHulls.Reserve(SampleCount - 1);
	for (int32 i = 1; i < SampleCount; i++)
	{
		TStaticArray<FVector, 4> NearCorners;
		TStaticArray<FVector, 4> FarCorners;
		for (int32 Corner = 0; Corner < 4; Corner++)
		{
			const TArray<FVector>& CornerPoints = Route.Path.Corners[Corner].Points;
			if (!CornerPoints.IsValidIndex(i))
			{
				return;
			}
			NearCorners[Corner] = CornerPoints[i - 1];
			FarCorners[Corner] = CornerPoints[i];
		}
		OutHulls.Add(FNRawMeshUtils::MakeConvexPrism(NearCorners, FarCorners));
	}
}

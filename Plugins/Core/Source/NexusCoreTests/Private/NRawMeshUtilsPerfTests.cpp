// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Types/NRawMeshUtils.h"
#include "Types/NRawMeshFactory.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness
{
	constexpr int32 SmallIterations = 10000;
	constexpr int32 MediumIterations = 1000;
	constexpr int32 LargeIterations = 100;
	constexpr int32 PointBatchCount = 100;
	constexpr int32 CombineMeshCount = 50;
	// MaxDuration values are in milliseconds and bound the total cost of the inner loop (NOT per call).
	// Sized to roughly 5x the observed baseline so the tests don't false-fail under CI / contended cores
	// while still flagging genuine regressions.
	constexpr float SmallMaxDuration = 50.0f;
	constexpr float MediumMaxDuration = 300.0f;
	constexpr float LargeMaxDuration = 3500.0f;

	/** 12-tri / 8-vert cube via FromChaosBox so FaceLoops is populated and bIsChaosGenerated is set. */
	static FNRawMesh MakeChaosBox(const double Full)
	{
		FKBoxElem Box;
		Box.X = Full;
		Box.Y = Full;
		Box.Z = Full;
		FNRawMesh OutMesh;
		FTransform OutTransform;
		FNRawMeshFactory::FromChaosBox(Box, FTransform::Identity, OutMesh, OutTransform);
		return OutMesh;
	}

	/**
	 * UV-sphere with controllable tessellation. Vertex count = 2 + (Rings-1) * Segments;
	 * triangle count = 2 * Segments + (Rings-2) * Segments * 2. Used as a stand-in for cell hulls
	 * (16x8 ≈ 224 tris) and dense world collision meshes (32x16 ≈ 960 tris).
	 */
	static FNRawMesh MakeSphere(const double Radius, const int32 Segments, const int32 Rings)
	{
		FNRawMesh Mesh;
		Mesh.Vertices.Reserve(2 + (Rings - 1) * Segments);
		Mesh.Vertices.Add(FVector(0.0, 0.0, Radius));
		for (int32 r = 1; r < Rings; ++r)
		{
			const double Phi = PI * r / Rings;
			const double Z = Radius * FMath::Cos(Phi);
			const double RingRadius = Radius * FMath::Sin(Phi);
			for (int32 s = 0; s < Segments; ++s)
			{
				const double Theta = 2.0 * PI * s / Segments;
				Mesh.Vertices.Add(FVector(RingRadius * FMath::Cos(Theta), RingRadius * FMath::Sin(Theta), Z));
			}
		}
		Mesh.Vertices.Add(FVector(0.0, 0.0, -Radius));
		const int32 BottomPole = Mesh.Vertices.Num() - 1;

		Mesh.Loops.Reserve(2 * Segments + (Rings - 2) * Segments * 2);
		for (int32 s = 0; s < Segments; ++s)
		{
			const int32 A = 1 + s;
			const int32 B = 1 + (s + 1) % Segments;
			Mesh.Loops.Add(FNRawMeshLoop(0, A, B));
		}
		for (int32 r = 0; r < Rings - 2; ++r)
		{
			const int32 RowStart = 1 + r * Segments;
			const int32 NextRowStart = RowStart + Segments;
			for (int32 s = 0; s < Segments; ++s)
			{
				const int32 A = RowStart + s;
				const int32 B = RowStart + (s + 1) % Segments;
				const int32 C = NextRowStart + (s + 1) % Segments;
				const int32 D = NextRowStart + s;
				Mesh.Loops.Add(FNRawMeshLoop(A, D, C));
				Mesh.Loops.Add(FNRawMeshLoop(A, C, B));
			}
		}
		const int32 LastRowStart = 1 + (Rings - 2) * Segments;
		for (int32 s = 0; s < Segments; ++s)
		{
			const int32 A = LastRowStart + s;
			const int32 B = LastRowStart + (s + 1) % Segments;
			Mesh.Loops.Add(FNRawMeshLoop(A, BottomPole, B));
		}

		Mesh.CalculateCenterAndBounds();
		Mesh.Validate();
		return Mesh;
	}

	/**
	 * Triangulated non-convex closed manifold (notched prism). Forces IsRelativePointInside / DoesIntersect down the
	 * parity-raycast fallback path. Mirrors the FNRawMeshUtilsHarness::MakeNonConvexPrism shape used in unit tests.
	 */
	static FNRawMesh MakeNonConvexPrism()
	{
		FNRawMesh Mesh;
		Mesh.Vertices = {
			{ 0,  0, 0 }, { 10,  0, 0 }, { 10, 10, 0 }, { 5,  5, 0 }, { 0, 10, 0 },
			{ 0,  0, 5 }, { 10,  0, 5 }, { 10, 10, 5 }, { 5,  5, 5 }, { 0, 10, 5 },
		};
		Mesh.Loops.Add(FNRawMeshLoop(3, 2, 1));
		Mesh.Loops.Add(FNRawMeshLoop(3, 1, 0));
		Mesh.Loops.Add(FNRawMeshLoop(3, 0, 4));
		Mesh.Loops.Add(FNRawMeshLoop(8, 9, 5));
		Mesh.Loops.Add(FNRawMeshLoop(8, 5, 6));
		Mesh.Loops.Add(FNRawMeshLoop(8, 6, 7));
		Mesh.Loops.Add(FNRawMeshLoop(0, 1, 6)); Mesh.Loops.Add(FNRawMeshLoop(0, 6, 5));
		Mesh.Loops.Add(FNRawMeshLoop(1, 2, 7)); Mesh.Loops.Add(FNRawMeshLoop(1, 7, 6));
		Mesh.Loops.Add(FNRawMeshLoop(2, 3, 8)); Mesh.Loops.Add(FNRawMeshLoop(2, 8, 7));
		Mesh.Loops.Add(FNRawMeshLoop(3, 4, 9)); Mesh.Loops.Add(FNRawMeshLoop(3, 9, 8));
		Mesh.Loops.Add(FNRawMeshLoop(4, 0, 5)); Mesh.Loops.Add(FNRawMeshLoop(4, 5, 9));
		Mesh.CalculateCenterAndBounds();
		Mesh.Validate();
		return Mesh;
	}
}

class FNRawMeshUtilsPerfTests
{
public:
	/** Box-vs-box DoesIntersect with the meshes overlapping. AABB pre-check passes; full tri-tri sweep runs. */
	static void DoesIntersect_BoxVsBox_Hit()
	{
		const FNRawMesh Left = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeChaosBox(10.0);
		const FNRawMesh Right = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeChaosBox(10.0);
		const FVector LeftOrigin(0.0, 0.0, 0.0);
		const FVector RightOrigin(5.0, 0.0, 0.0);
		const FRotator NoRot = FRotator::ZeroRotator;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_DoesIntersect_BoxVsBox_Hit,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::SmallMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::SmallIterations; ++i)
			{
				FNRawMeshUtils::DoesIntersect(Left, LeftOrigin, NoRot, Right, RightOrigin, NoRot);
			}
			NTestTimer.ManualStop();
		}
	}

	/** Box-vs-box DoesIntersect with the AABBs disjoint — the AABB early-out is the entire cost. */
	static void DoesIntersect_BoxVsBox_AABBMiss()
	{
		const FNRawMesh Left = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeChaosBox(10.0);
		const FNRawMesh Right = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeChaosBox(10.0);
		const FVector LeftOrigin(0.0, 0.0, 0.0);
		const FVector RightOrigin(1000.0, 0.0, 0.0);
		const FRotator NoRot = FRotator::ZeroRotator;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_DoesIntersect_BoxVsBox_AABBMiss,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::SmallMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::SmallIterations; ++i)
			{
				FNRawMeshUtils::DoesIntersect(Left, LeftOrigin, NoRot, Right, RightOrigin, NoRot);
			}
			NTestTimer.ManualStop();
		}
	}

	/**
	 * Box-vs-box where AABBs overlap but the meshes don't actually touch — the SAT face-normal
	 * early-out case. Two boxes can't directly produce this configuration (axis-aligned + rotated
	 * always yields mesh overlap whenever AABBs overlap), so we use one large rotated box and one
	 * small box parked outside the rotated box's nearest face. Both have FaceLoops populated so the
	 * SAT path is exercised.
	 */
	static void DoesIntersect_BoxVsBox_AABBHitMeshMiss()
	{
		const FNRawMesh Left = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeChaosBox(10.0);
		const FNRawMesh Right = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeChaosBox(2.0);
		const FVector LeftOrigin(0.0, 0.0, 0.0);
		// Left rotated 45 deg yaw extends its AABB out to (~7.07, 7.07, 5). Right at (6, 6, 0) puts
		// the small box's [5..7, 5..7] AABB squarely inside Left's AABB envelope, but Left's nearest
		// rotated face is the line x + y = 7.07 — every Right corner satisfies x + y >= 10 and so
		// lives outside Left's actual hull.
		const FVector RightOrigin(6.0, 6.0, 0.0);
		const FRotator LeftRot(0.0, 45.0, 0.0);
		const FRotator RightRot = FRotator::ZeroRotator;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_DoesIntersect_BoxVsBox_AABBHitMeshMiss,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::SmallMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::SmallIterations; ++i)
			{
				FNRawMeshUtils::DoesIntersect(Left, LeftOrigin, LeftRot, Right, RightOrigin, RightRot);
			}
			NTestTimer.ManualStop();
		}
	}

	/** Medium-hull-vs-medium-hull overlap — exercises the O(L * R) triangle pair sweep at ~50k pairs. */
	static void DoesIntersect_HullVsHull_Hit()
	{
		const FNRawMesh Left = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(50.0, 16, 8);
		const FNRawMesh Right = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(50.0, 16, 8);
		const FVector LeftOrigin(0.0, 0.0, 0.0);
		const FVector RightOrigin(30.0, 0.0, 0.0);
		const FRotator NoRot = FRotator::ZeroRotator;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_DoesIntersect_HullVsHull_Hit,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MediumIterations; ++i)
			{
				FNRawMeshUtils::DoesIntersect(Left, LeftOrigin, NoRot, Right, RightOrigin, NoRot);
			}
			NTestTimer.ManualStop();
		}
	}

	/** Medium-hull-vs-medium-hull, AABB-disjoint — sanity check that the early-out scales the same as boxes. */
	static void DoesIntersect_HullVsHull_AABBMiss()
	{
		const FNRawMesh Left = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(50.0, 16, 8);
		const FNRawMesh Right = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(50.0, 16, 8);
		const FVector LeftOrigin(0.0, 0.0, 0.0);
		const FVector RightOrigin(1000.0, 0.0, 0.0);
		const FRotator NoRot = FRotator::ZeroRotator;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_DoesIntersect_HullVsHull_AABBMiss,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::SmallMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::SmallIterations; ++i)
			{
				FNRawMeshUtils::DoesIntersect(Left, LeftOrigin, NoRot, Right, RightOrigin, NoRot);
			}
			NTestTimer.ManualStop();
		}
	}

	/**
	 * Worst-case DoesIntersect: AABBs overlap so the early-out can't fire, but the hulls don't actually
	 * touch — every tri-tri pair runs and fails, then the containment fallback runs both ways.
	 */
	static void DoesIntersect_HullVsHull_AABBHitMeshMiss()
	{
		const FNRawMesh Left = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(20.0, 16, 8);
		const FNRawMesh Right = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(20.0, 16, 8);
		// Offset so the bounding spheres of radius 20 are at distance 50 — AABBs overlap (each AABB is
		// roughly [-20, +20]; centres 50 apart leaves AABB[-20..20] vs AABB[30..70] disjoint... so push
		// them closer to force AABB overlap without surface contact).
		const FVector LeftOrigin(0.0, 0.0, 0.0);
		const FVector RightOrigin(35.0, 0.0, 0.0);
		const FRotator NoRot = FRotator::ZeroRotator;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_DoesIntersect_HullVsHull_AABBHitMeshMiss,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MediumIterations; ++i)
			{
				FNRawMeshUtils::DoesIntersect(Left, LeftOrigin, NoRot, Right, RightOrigin, NoRot);
			}
			NTestTimer.ManualStop();
		}
	}

	/** GetIntersectDepth on overlapping medium hulls — exercises symmetric per-vertex containment + depth. */
	static void GetIntersectDepth_HullVsHull_Overlap()
	{
		const FNRawMesh Left = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(50.0, 16, 8);
		const FNRawMesh Right = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(50.0, 16, 8);
		const FVector LeftOrigin(0.0, 0.0, 0.0);
		const FVector RightOrigin(30.0, 0.0, 0.0);
		const FRotator NoRot = FRotator::ZeroRotator;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_GetIntersectDepth_HullVsHull_Overlap,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MediumIterations; ++i)
			{
				FNRawMeshUtils::GetIntersectDepth(Left, LeftOrigin, NoRot, Right, RightOrigin, NoRot);
			}
			NTestTimer.ManualStop();
		}
	}

	/**
	 * Same overlapping hulls as GetIntersectDepth_HullVsHull_Overlap, but with a tight EarlyExitDepth
	 * so the in-loop A short-circuit fires after the first deep vertex. Exercises path A.
	 */
	static void GetIntersectDepth_HullVsHull_Overlap_EarlyExit()
	{
		const FNRawMesh Left = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(50.0, 16, 8);
		const FNRawMesh Right = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(50.0, 16, 8);
		const FVector LeftOrigin(0.0, 0.0, 0.0);
		const FVector RightOrigin(30.0, 0.0, 0.0);
		const FRotator NoRot = FRotator::ZeroRotator;
		// Threshold deliberately tiny — any deep vertex satisfies it on the first hit.
		constexpr float EarlyExitDepth = 5.0f;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_GetIntersectDepth_HullVsHull_Overlap_EarlyExit,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MediumIterations; ++i)
			{
				FNRawMeshUtils::GetIntersectDepth(Left, LeftOrigin, NoRot, Right, RightOrigin, NoRot, EarlyExitDepth);
			}
			NTestTimer.ManualStop();
		}
	}

	/**
	 * Same overlapping hulls but with a loose EarlyExitDepth above the AABB-overlap depth bound. The
	 * C optimisation returns the conservative bound without sampling any vertices. Exercises path C.
	 */
	static void GetIntersectDepth_HullVsHull_Overlap_AABBBound()
	{
		const FNRawMesh Left = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(50.0, 16, 8);
		const FNRawMesh Right = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(50.0, 16, 8);
		const FVector LeftOrigin(0.0, 0.0, 0.0);
		const FVector RightOrigin(30.0, 0.0, 0.0);
		const FRotator NoRot = FRotator::ZeroRotator;
		// AABB overlap on the X axis is 70 (offset 30 plus the 100-wide AABBs); a threshold of 100
		// is above 70 so C proves "depth is at most 70" without touching a single vertex.
		constexpr float EarlyExitDepth = 100.0f;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_GetIntersectDepth_HullVsHull_Overlap_AABBBound,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::SmallMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MediumIterations; ++i)
			{
				FNRawMeshUtils::GetIntersectDepth(Left, LeftOrigin, NoRot, Right, RightOrigin, NoRot, EarlyExitDepth);
			}
			NTestTimer.ManualStop();
		}
	}

	/**
	 * GetIntersectDepth dense-vs-dense — closest analogue to "graph-builder cell vs large world collision mesh".
	 * Each call does ~480 verts x ~224 faces both ways via the convex face-plane fast path.
	 */
	static void GetIntersectDepth_HullVsWorldMesh()
	{
		const FNRawMesh Hull = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(50.0, 16, 8);
		const FNRawMesh WorldMesh = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(75.0, 32, 16);
		const FVector LeftOrigin(0.0, 0.0, 0.0);
		const FVector RightOrigin(40.0, 0.0, 0.0);
		const FRotator NoRot = FRotator::ZeroRotator;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_GetIntersectDepth_HullVsWorldMesh,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::LargeMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::LargeIterations; ++i)
			{
				FNRawMeshUtils::GetIntersectDepth(Hull, LeftOrigin, NoRot, WorldMesh, RightOrigin, NoRot);
			}
			NTestTimer.ManualStop();
		}
	}

	/** IsRelativePointInside on a convex medium hull — exercises the face-plane fast path per query. */
	static void IsRelativePointInside_Convex()
	{
		const FNRawMesh Mesh = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(50.0, 16, 8);
		const FVector Point(5.0, 5.0, 5.0);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_IsRelativePointInside_Convex,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::SmallIterations; ++i)
			{
				FNRawMeshUtils::IsRelativePointInside(Mesh, Point);
			}
			NTestTimer.ManualStop();
		}
	}

	/** IsRelativePointInside on a non-convex prism — exercises the parity-raycast fallback over every triangle. */
	static void IsRelativePointInside_NonConvex()
	{
		const FNRawMesh Mesh = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeNonConvexPrism();
		const FVector Point(3.0, 3.0, 2.5);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_IsRelativePointInside_NonConvex,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::SmallIterations; ++i)
			{
				FNRawMeshUtils::IsRelativePointInside(Mesh, Point);
			}
			NTestTimer.ManualStop();
		}
	}

	/** AnyRelativePointsInside hitting on the first probe — measures short-circuit overhead. */
	static void AnyRelativePointsInside_FirstHit()
	{
		const FNRawMesh Mesh = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(50.0, 16, 8);
		TArray<FVector> Points;
		Points.Reserve(NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::PointBatchCount);
		Points.Add(FVector(0.0, 0.0, 0.0));
		for (int32 i = 1; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::PointBatchCount; ++i)
		{
			Points.Add(FVector(1000.0 + i, 0.0, 0.0));
		}

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_AnyRelativePointsInside_FirstHit,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MediumIterations; ++i)
			{
				FNRawMeshUtils::AnyRelativePointsInside(Mesh, Points);
			}
			NTestTimer.ManualStop();
		}
	}

	/** AnyRelativePointsInside that never hits — every point pays the full convex face-plane sweep. */
	static void AnyRelativePointsInside_NoHit()
	{
		const FNRawMesh Mesh = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(50.0, 16, 8);
		TArray<FVector> Points;
		Points.Reserve(NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::PointBatchCount);
		for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::PointBatchCount; ++i)
		{
			Points.Add(FVector(1000.0 + i, 0.0, 0.0));
		}

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_AnyRelativePointsInside_NoHit,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MediumIterations; ++i)
			{
				FNRawMeshUtils::AnyRelativePointsInside(Mesh, Points);
			}
			NTestTimer.ManualStop();
		}
	}

	/**
	 * CombineMesh in a tight accumulation loop, mirroring the single-actor visualizer path. Each Combine
	 * triggers Validate on the growing base mesh — the O(N²) cumulative cost this scenario exposes.
	 */
	static void CombineMesh_AccumulateMany()
	{
		const FNRawMesh Source = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeChaosBox(10.0);
		const FTransform SourceTransform(FRotator::ZeroRotator, FVector(15.0, 0.0, 0.0));
		const FTransform BaseTransform = FTransform::Identity;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_CombineMesh_AccumulateMany,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::LargeMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::LargeIterations; ++i)
			{
				FNRawMesh Base;
				for (int32 j = 0; j < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::CombineMeshCount; ++j)
				{
					FNRawMeshUtils::CombineMesh(BaseTransform, Base, SourceTransform, Source);
				}
			}
			NTestTimer.ManualStop();
		}
	}

	/** ToConvexHull on a dense point cloud — exercises Chaos's convex builder + the post-build triangulation. */
	static void ToConvexHull_PointCloud()
	{
		FNRawMesh Source = NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::MakeSphere(50.0, 32, 16);
		// Strip topology so IsConvex returns false on the source and ToConvexHull actually runs the builder.
		Source.Loops.Reset();
		Source.FaceLoops.Reset();
		Source.Validate();

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshUtilsPerfTests_ToConvexHull_PointCloud,
				NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::LargeMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshUtilsHarness::LargeIterations; ++i)
			{
				FNRawMesh Hull = FNRawMeshUtils::ToConvexHull(Source);
			}
			NTestTimer.ManualStop();
		}
	}
};

N_TEST_PERF(FNRawMeshUtilsPerfTests_DoesIntersect_BoxVsBox_Hit,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::DoesIntersect_BoxVsBox_Hit",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::DoesIntersect_BoxVsBox_Hit));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshUtilsPerfTests_DoesIntersect_BoxVsBox_AABBMiss,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::DoesIntersect_BoxVsBox_AABBMiss",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::DoesIntersect_BoxVsBox_AABBMiss));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshUtilsPerfTests_DoesIntersect_HullVsHull_Hit,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::DoesIntersect_HullVsHull_Hit",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::DoesIntersect_HullVsHull_Hit));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshUtilsPerfTests_DoesIntersect_BoxVsBox_AABBHitMeshMiss,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::DoesIntersect_BoxVsBox_AABBHitMeshMiss",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::DoesIntersect_BoxVsBox_AABBHitMeshMiss));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshUtilsPerfTests_DoesIntersect_HullVsHull_AABBMiss,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::DoesIntersect_HullVsHull_AABBMiss",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::DoesIntersect_HullVsHull_AABBMiss));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshUtilsPerfTests_DoesIntersect_HullVsHull_AABBHitMeshMiss,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::DoesIntersect_HullVsHull_AABBHitMeshMiss",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::DoesIntersect_HullVsHull_AABBHitMeshMiss));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshUtilsPerfTests_GetIntersectDepth_HullVsHull_Overlap,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::GetIntersectDepth_HullVsHull_Overlap",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::GetIntersectDepth_HullVsHull_Overlap));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshUtilsPerfTests_GetIntersectDepth_HullVsWorldMesh,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::GetIntersectDepth_HullVsWorldMesh",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::GetIntersectDepth_HullVsWorldMesh));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshUtilsPerfTests_GetIntersectDepth_HullVsHull_Overlap_EarlyExit,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::GetIntersectDepth_HullVsHull_Overlap_EarlyExit",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::GetIntersectDepth_HullVsHull_Overlap_EarlyExit));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshUtilsPerfTests_GetIntersectDepth_HullVsHull_Overlap_AABBBound,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::GetIntersectDepth_HullVsHull_Overlap_AABBBound",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::GetIntersectDepth_HullVsHull_Overlap_AABBBound));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshUtilsPerfTests_IsRelativePointInside_Convex,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::IsRelativePointInside_Convex",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::IsRelativePointInside_Convex));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshUtilsPerfTests_IsRelativePointInside_NonConvex,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::IsRelativePointInside_NonConvex",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::IsRelativePointInside_NonConvex));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshUtilsPerfTests_AnyRelativePointsInside_FirstHit,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::AnyRelativePointsInside_FirstHit",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::AnyRelativePointsInside_FirstHit));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshUtilsPerfTests_AnyRelativePointsInside_NoHit,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::AnyRelativePointsInside_NoHit",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::AnyRelativePointsInside_NoHit));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshUtilsPerfTests_CombineMesh_AccumulateMany,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::CombineMesh_AccumulateMany",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::CombineMesh_AccumulateMany));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshUtilsPerfTests_ToConvexHull_PointCloud,
	"NEXUS::PerfTests::NCore::FNRawMeshUtils::ToConvexHull_PointCloud",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Setup strips Loops/FaceLoops so ToConvexHull takes the Chaos-builder path; the subsequent Validate
	// then fires CheckConvex's "no vertices or loops" warning by design — silence it like the unit test does.
	AddExpectedMessage(TEXT("No vertices or loops were found in the FNRawMesh"), ELogVerbosity::Warning);
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshUtilsPerfTests::ToConvexHull_PointCloud));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

#endif //WITH_TESTS

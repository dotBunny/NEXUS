// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldAssemblyUtils.h"
#include "Cell/NCellHullGenerationSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Developer/NTestUtils.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Macros/NTestMacros.h"
#include "Math/NVectorUtils.h"
#include "Tests/TestHarnessAdapter.h"
#include "Types/NRawMesh.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtilsHarness
{
	/** Tolerance for geometry comparisons; generous enough to absorb the slab method's float error. */
	constexpr double Tolerance = 1.e-3;

	/** @return true if Points contains a vector within Tolerance of Target. */
	static bool Contains(const TArray<FVector>& Points, const FVector& Target)
	{
		for (const FVector& Point : Points)
		{
			if (Point.Equals(Target, Tolerance)) return true;
		}
		return false;
	}

	/** @return true if Points contains a vector within Tolerance of Target. */
	static bool Contains(const TArray<FVector2D>& Points, const FVector2D& Target)
	{
		for (const FVector2D& Point : Points)
		{
			if (Point.Equals(Target, Tolerance)) return true;
		}
		return false;
	}

	/** @return the average of all supplied points; used to assert a point set is centered on the origin. */
	static FVector2D Centroid(const TArray<FVector2D>& Points)
	{
		FVector2D Sum = FVector2D::ZeroVector;
		for (const FVector2D& Point : Points) Sum += Point;
		return Points.Num() > 0 ? Sum / Points.Num() : Sum;
	}

	static FVector Centroid(const TArray<FVector>& Points)
	{
		FVector Sum = FVector::ZeroVector;
		for (const FVector& Point : Points) Sum += Point;
		return Points.Num() > 0 ? Sum / Points.Num() : Sum;
	}

	/** @return true if P lies within Tolerance of the surface of Box (inside-or-on, touching at least one face). */
	static bool OnBoxSurface(const FBox& Box, const FVector& P)
	{
		if (!Box.ExpandBy(Tolerance).IsInsideOrOn(P)) return false;
		const bool bTouchesFace =
			FMath::IsNearlyEqual(P.X, Box.Min.X, Tolerance) || FMath::IsNearlyEqual(P.X, Box.Max.X, Tolerance) ||
			FMath::IsNearlyEqual(P.Y, Box.Min.Y, Tolerance) || FMath::IsNearlyEqual(P.Y, Box.Max.Y, Tolerance) ||
			FMath::IsNearlyEqual(P.Z, Box.Min.Z, Tolerance) || FMath::IsNearlyEqual(P.Z, Box.Max.Z, Tolerance);
		return bTouchesFace;
	}
}

//
// RayAABBIntersection
//

N_TEST_HIGH(FNWorldAssemblyUtilsTests_RayAABBIntersection_HitFromOutside,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::RayAABBIntersection::HitFromOutside",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FBox Box(FVector(-1.0), FVector(1.0));
	FVector Intersection;
	const bool bHit = FNWorldAssemblyUtils::RayAABBIntersection(FVector(-10, 0, 0), FVector(1, 0, 0), Box, Intersection);

	CHECK_MESSAGE(TEXT("A ray aimed at the box from outside should report a hit."), bHit)
	CHECK_MESSAGE(TEXT("Intersection should land on the near (-X) face."),
		Intersection.Equals(FVector(-1, 0, 0), NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtilsHarness::Tolerance))
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_RayAABBIntersection_MissPointingAway,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::RayAABBIntersection::MissPointingAway",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FBox Box(FVector(-1.0), FVector(1.0));
	FVector Intersection;
	// Origin outside, direction pointing away: the only solutions are behind the ray (tFar < 0).
	const bool bHit = FNWorldAssemblyUtils::RayAABBIntersection(FVector(-10, 0, 0), FVector(-1, 0, 0), Box, Intersection);

	CHECK_FALSE_MESSAGE(TEXT("A ray pointing away from the box must not report a hit."), bHit)
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_RayAABBIntersection_MissParallelOffset,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::RayAABBIntersection::MissParallelOffset",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FBox Box(FVector(-1.0), FVector(1.0));
	FVector Intersection;
	// Parallel to X but offset out of the Y slab: exercises the inf/-inf slab branch without a true hit.
	const bool bHit = FNWorldAssemblyUtils::RayAABBIntersection(FVector(-10, 5, 0), FVector(1, 0, 0), Box, Intersection);

	CHECK_FALSE_MESSAGE(TEXT("A ray running parallel to the box but offset outside it must not report a hit."), bHit)
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_RayAABBIntersection_OriginInside,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::RayAABBIntersection::OriginInside",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FBox Box(FVector(-1.0), FVector(1.0));
	FVector Intersection;
	// Origin inside: tNear is negative, so the method must fall back to the exit point (tFar).
	const bool bHit = FNWorldAssemblyUtils::RayAABBIntersection(FVector(0, 0, 0), FVector(1, 0, 0), Box, Intersection);

	CHECK_MESSAGE(TEXT("A ray originating inside the box should report a hit."), bHit)
	CHECK_MESSAGE(TEXT("Intersection should be the exit point on the far (+X) face."),
		Intersection.Equals(FVector(1, 0, 0), NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtilsHarness::Tolerance))
}

//
// GetVoxelQueryPoints
//

N_TEST_HIGH(FNWorldAssemblyUtilsTests_GetVoxelQueryPoints_EmitsTwentySixUniqueNeighbours,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::GetVoxelQueryPoints::EmitsTwentySixUniqueNeighbours",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the 26-neighbour contract: one sample per surrounding voxel (faces + edges + corners),
	// the center itself excluded, with no duplicates.
	const FVector Center(10, 20, 30);
	const FVector VoxelSize(2, 3, 4);

	TArray<FVector> Points;
	FNWorldAssemblyUtils::GetVoxelQueryPoints(Center, VoxelSize, Points);

	CHECK_EQUALS("Should emit exactly VoxelQueryPointCount samples.", Points.Num(), static_cast<int32>(FNWorldAssemblyUtils::VoxelQueryPointCount))
	CHECK_EQUALS("VoxelQueryPointCount should be 26.", static_cast<int32>(FNWorldAssemblyUtils::VoxelQueryPointCount), 26)

	const TSet<FVector> Unique(Points);
	CHECK_EQUALS("All emitted neighbour samples should be unique.", Unique.Num(), Points.Num())

	CHECK_FALSE_MESSAGE(TEXT("The center point must not be emitted as one of the neighbours."),
		NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtilsHarness::Contains(Points, Center))
}

N_TEST_MEDIUM(FNWorldAssemblyUtilsTests_GetVoxelQueryPoints_OffsetByVoxelSize,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::GetVoxelQueryPoints::OffsetByVoxelSize",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtilsHarness;
	const FVector Center(10, 20, 30);
	const FVector VoxelSize(2, 3, 4);

	TArray<FVector> Points;
	FNWorldAssemblyUtils::GetVoxelQueryPoints(Center, VoxelSize, Points);

	// A face neighbour (+X only) and the opposite corner neighbour (-X,-Y,-Z) must both be present.
	CHECK_MESSAGE(TEXT("The +X face neighbour should be Center offset by VoxelSize on X."),
		Contains(Points, Center + FVector(VoxelSize.X, 0, 0)))
	CHECK_MESSAGE(TEXT("The (-X,-Y,-Z) corner neighbour should be present."),
		Contains(Points, Center + FVector(-VoxelSize.X, -VoxelSize.Y, -VoxelSize.Z)))
}

//
// GetVoxelQueryLevelBoundsEndPoints
//

N_TEST_MEDIUM(FNWorldAssemblyUtilsTests_GetVoxelQueryLevelBoundsEndPoints_LandOnExpandedBounds,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::GetVoxelQueryLevelBoundsEndPoints::LandOnExpandedBounds",
	N_TEST_CONTEXT_ANYWHERE)
{
	// From a center inside the level bounds every neighbour direction should cast out to the (10cm-expanded)
	// bounds surface, producing 26 endpoints that all sit on that surface.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtilsHarness;
	const FVector Center = FVector::ZeroVector;
	const FBox LevelBounds(FVector(-100.0), FVector(100.0));
	const FBox ExpandedBounds = LevelBounds.ExpandBy(10.f);

	TArray<FVector> Points;
	FNWorldAssemblyUtils::GetVoxelQueryLevelBoundsEndPoints(Center, LevelBounds, Points);

	CHECK_EQUALS("Should produce one endpoint per neighbour direction.", Points.Num(), 26)

	bool bAllOnSurface = true;
	for (const FVector& Point : Points)
	{
		if (!OnBoxSurface(ExpandedBounds, Point))
		{
			bAllOnSurface = false;
			break;
		}
	}
	CHECK_MESSAGE(TEXT("Every endpoint should land on the expanded level-bounds surface."), bAllOnSurface)
}

//
// GetSocketPoints2D
//

N_TEST_HIGH(FNWorldAssemblyUtilsTests_GetSocketPoints2D_SingleUnitCentered,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::GetSocketPoints2D::SingleUnitCentered",
	N_TEST_CONTEXT_ANYWHERE)
{
	const TArray<FVector2D> Points = FNWorldAssemblyUtils::GetSocketPoints2D(FIntVector2(1, 1), FVector2D(100, 100));

	CHECK_EQUALS("A 1x1 socket should produce a single point.", Points.Num(), 1)
	CHECK_MESSAGE(TEXT("The single point should sit at the origin."),
		Points[0].Equals(FVector2D::ZeroVector, NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtilsHarness::Tolerance))
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_GetSocketPoints2D_GridCountAndCentered,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::GetSocketPoints2D::GridCountAndCentered",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtilsHarness;
	const TArray<FVector2D> Points = FNWorldAssemblyUtils::GetSocketPoints2D(FIntVector2(2, 2), FVector2D(10, 10));

	CHECK_EQUALS("A 2x2 socket should produce four points.", Points.Num(), 4)
	CHECK_MESSAGE(TEXT("A 2x2 socket should be centered on the origin."),
		Centroid(Points).Equals(FVector2D::ZeroVector, Tolerance))
	CHECK_MESSAGE(TEXT("Corner (-5,-5) should be present."), Contains(Points, FVector2D(-5, -5)))
	CHECK_MESSAGE(TEXT("Corner (5,5) should be present."), Contains(Points, FVector2D(5, 5)))

	// Count is the product of the unit dimensions regardless of unit size.
	const TArray<FVector2D> Wide = FNWorldAssemblyUtils::GetSocketPoints2D(FIntVector2(3, 4), FVector2D(10, 5));
	CHECK_EQUALS("Point count should equal Units.X * Units.Y.", Wide.Num(), 12)
}

//
// GetCenteredWorldCornerPoints2D
//

N_TEST_HIGH(FNWorldAssemblyUtilsTests_GetCenteredWorldCornerPoints2D_ZAxisLayout,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::GetCenteredWorldCornerPoints2D::ZAxisLayout",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtilsHarness;
	const TArray<FVector> Points = FNWorldAssemblyUtils::GetCenteredWorldCornerPoints2D(20.f, 10.f, ENAxis::Z);

	CHECK_EQUALS("Should produce four corners.", Points.Num(), 4)
	CHECK_MESSAGE(TEXT("Z-axis corners should lie in the X-Z plane (Y == 0)."),
		Centroid(Points).Equals(FVector::ZeroVector, Tolerance) &&
		FMath::IsNearlyZero(Points[0].Y, Tolerance) && FMath::IsNearlyZero(Points[2].Y, Tolerance))
	CHECK_MESSAGE(TEXT("Top-left corner should be (-10, 0, 5)."), Contains(Points, FVector(-10, 0, 5)))
	CHECK_MESSAGE(TEXT("Bottom-right corner should be (10, 0, -5)."), Contains(Points, FVector(10, 0, -5)))
}

N_TEST_MEDIUM(FNWorldAssemblyUtilsTests_GetCenteredWorldCornerPoints2D_AxisVariants,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::GetCenteredWorldCornerPoints2D::AxisVariants",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtilsHarness;

	const TArray<FVector> YPoints = FNWorldAssemblyUtils::GetCenteredWorldCornerPoints2D(20.f, 10.f, ENAxis::Y);
	CHECK_MESSAGE(TEXT("Y-axis corners should lie in the X-Y plane (Z == 0) and stay centered."),
		Centroid(YPoints).Equals(FVector::ZeroVector, Tolerance) &&
		FMath::IsNearlyZero(YPoints[0].Z, Tolerance) && FMath::IsNearlyZero(YPoints[3].Z, Tolerance))
	CHECK_MESSAGE(TEXT("Y-axis should contain corner (10, 5, 0)."), Contains(YPoints, FVector(10, 5, 0)))

	const TArray<FVector> XPoints = FNWorldAssemblyUtils::GetCenteredWorldCornerPoints2D(20.f, 10.f, ENAxis::X);
	CHECK_MESSAGE(TEXT("X-axis corners should stay centered on the origin."),
		Centroid(XPoints).Equals(FVector::ZeroVector, Tolerance))
	CHECK_MESSAGE(TEXT("X-axis should contain corner (5, 0, -10)."), Contains(XPoints, FVector(5, 0, -10)))
}

//
// GetWorldSize2D / OffsetLocation / CreateRotatedBox
//

N_TEST_MEDIUM(FNWorldAssemblyUtilsTests_GetWorldSize2D_ScalesByUnitSize,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::GetWorldSize2D::ScalesByUnitSize",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtilsHarness;
	const FIntVector2 Units(3, 4);

	const FVector2D From2D = FNWorldAssemblyUtils::GetWorldSize2D(Units, FVector2D(10, 20));
	CHECK_MESSAGE(TEXT("FVector2D overload should scale each axis by its unit size."),
		From2D.Equals(FVector2D(30, 80), Tolerance))

	const FVector2D From3D = FNWorldAssemblyUtils::GetWorldSize2D(Units, FVector(10, 20, 5));
	CHECK_MESSAGE(TEXT("FVector overload should use only the XY components."),
		From3D.Equals(FVector2D(30, 80), Tolerance))
}

N_TEST_MEDIUM(FNWorldAssemblyUtilsTests_OffsetLocation_RotatesThenTranslates,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::OffsetLocation::RotatesThenTranslates",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtilsHarness;
	// A 90-degree yaw rotates +X onto +Y, then the offset is added.
	const FVector Result = FNWorldAssemblyUtils::OffsetLocation(FVector(1, 0, 0), FRotator(0, 90, 0), FVector(5, 5, 5));
	CHECK_MESSAGE(TEXT("OffsetLocation should rotate the input then translate by the offset."),
		Result.Equals(FVector(5, 6, 5), Tolerance))
}

N_TEST_MEDIUM(FNWorldAssemblyUtilsTests_CreateRotatedBox_TranslatesByOffset,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::CreateRotatedBox::TranslatesByOffset",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtilsHarness;
	const FBox Box(FVector(-1.0), FVector(1.0));
	const FBox Result = FNWorldAssemblyUtils::CreateRotatedBox(Box, FRotator::ZeroRotator, FVector(10, 0, 0));

	CHECK_MESSAGE(TEXT("A zero-rotation box should simply translate by the offset (min)."),
		Result.Min.Equals(FVector(9, -1, -1), Tolerance))
	CHECK_MESSAGE(TEXT("A zero-rotation box should simply translate by the offset (max)."),
		Result.Max.Equals(FVector(11, 1, 1), Tolerance))
}

//
// CalculateConvexHull
//

N_TEST_HIGH(FNWorldAssemblyUtilsTests_CalculateConvexHull_NullLevel_EmptyMesh,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::CalculateConvexHull::NullLevel_EmptyMesh",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Null level is the early-out guard — it must return a default, empty mesh without touching the registry.
	const FNCellHullGenerationSettings Settings;
	const FNRawMesh Mesh = FNWorldAssemblyUtils::CalculateConvexHull(nullptr, Settings);

	CHECK_EQUALS("A null level should yield no hull vertices", Mesh.Vertices.Num(), 0)
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_CalculateConvexHull_StaticMeshActor_ProducesConvexHull,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::CalculateConvexHull::StaticMeshActor_ProducesConvexHull",
	N_TEST_CONTEXT_EDITOR)
{
	// A single 100cm engine cube must drive a convex hull built from its collision geometry rather than an inflated
	// render-bounds box. The hull should be convex, carry bounds, and stay close to the cube's ±50 extents.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
		if (Cube == nullptr)
		{
			ADD_ERROR("Failed to load /Engine/BasicShapes/Cube");
			return;
		}

		AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>();
		if (MeshActor == nullptr)
		{
			ADD_ERROR("Failed to spawn AStaticMeshActor");
			return;
		}
		UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->SetStaticMesh(Cube);

		const FNCellHullGenerationSettings Settings;
		const FNRawMesh Mesh = FNWorldAssemblyUtils::CalculateConvexHull(World->PersistentLevel, Settings);

		CHECK_MESSAGE(TEXT("The cube actor should drive a hull with at least the cube's 8 corners"),
			Mesh.Vertices.Num() >= 8)
		CHECK_MESSAGE(TEXT("The generated hull should report bounds"), Mesh.HasBounds())
		CHECK_MESSAGE(TEXT("A Chaos-built hull over a single convex cube should be convex"), Mesh.IsConvex())
		CHECK_MESSAGE(TEXT("Hull extents should match the cube's collision geometry, not an inflated box"),
			Mesh.Bounds.Max.X < 100.0 && Mesh.Bounds.Min.X > -100.0)
	});
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_CalculateConvexHull_IgnoreTag_ExcludesActor,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::CalculateConvexHull::IgnoreTag_ExcludesActor",
	N_TEST_CONTEXT_EDITOR)
{
	// An actor carrying one of Settings.ActorIgnoreTags must be filtered out before geometry extraction. With the
	// only actor ignored, the hull collapses to nothing — locking the Step 1 tag filter.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
		if (Cube == nullptr)
		{
			ADD_ERROR("Failed to load /Engine/BasicShapes/Cube");
			return;
		}

		AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>();
		if (MeshActor == nullptr)
		{
			ADD_ERROR("Failed to spawn AStaticMeshActor");
			return;
		}
		UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->SetStaticMesh(Cube);

		FNCellHullGenerationSettings Settings;
		MeshActor->Tags.Add(Settings.ActorIgnoreTags[0]);

		const FNRawMesh Mesh = FNWorldAssemblyUtils::CalculateConvexHull(World->PersistentLevel, Settings);

		CHECK_EQUALS("An ignored actor should contribute no hull vertices", Mesh.Vertices.Num(), 0)
	});
}

#endif //WITH_TESTS

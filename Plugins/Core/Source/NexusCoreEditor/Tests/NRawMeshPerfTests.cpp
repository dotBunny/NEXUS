// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Types/NRawMesh.h"
#include "Types/NRawMeshFactory.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NCore::FNRawMeshHarness
{
	constexpr int32 SmallIterations = 10000;
	constexpr int32 MediumIterations = 1000;
	constexpr int32 LargeIterations = 100;
	// MaxDuration values are in milliseconds and bound the total cost of the inner loop (NOT per call).
	// Sized to roughly 5x the observed baseline so the tests don't false-fail under CI / contended cores
	// while still flagging genuine regressions.
	constexpr float SmallMaxDuration = 50.0f;
	constexpr float MediumMaxDuration = 300.0f;
	constexpr float LargeMaxDuration = 1000.0f;

	/** Builds a 12-tri / 8-vert cube via FromChaosBox so FaceLoops is populated (matches the common production path). */
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
	 * Builds a UV-sphere FNRawMesh at the requested tessellation, returning a fully triangulated convex mesh whose
	 * vertex / loop counts grow predictably with Segments and Rings. Used to size perf inputs without relying on the
	 * Chaos factory defaults.
	 *
	 * Vertex count: 2 + (Rings - 1) * Segments. Triangle count: 2 * Segments + (Rings - 2) * Segments * 2.
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

	/** Builds a convex hexagonal prism with two 6-gon FaceLoops + 6 quad sides — forces ConvertToTriangles down the fan path. */
	static FNRawMesh MakeNgonPrism(const int32 Sides, const double Radius, const double Height)
	{
		FNRawMesh Mesh;
		Mesh.Vertices.Reserve(Sides * 2);
		for (int32 i = 0; i < Sides; ++i)
		{
			const double Theta = 2.0 * PI * i / Sides;
			Mesh.Vertices.Add(FVector(Radius * FMath::Cos(Theta), Radius * FMath::Sin(Theta), -Height * 0.5));
		}
		for (int32 i = 0; i < Sides; ++i)
		{
			const double Theta = 2.0 * PI * i / Sides;
			Mesh.Vertices.Add(FVector(Radius * FMath::Cos(Theta), Radius * FMath::Sin(Theta), +Height * 0.5));
		}

		TArray<int32> Bottom;
		Bottom.Reserve(Sides);
		for (int32 i = Sides - 1; i >= 0; --i) Bottom.Add(i);
		Mesh.Loops.Add(FNRawMeshLoop(MoveTemp(Bottom)));

		TArray<int32> Top;
		Top.Reserve(Sides);
		for (int32 i = 0; i < Sides; ++i) Top.Add(Sides + i);
		Mesh.Loops.Add(FNRawMeshLoop(MoveTemp(Top)));

		for (int32 i = 0; i < Sides; ++i)
		{
			const int32 A = i;
			const int32 B = (i + 1) % Sides;
			const int32 C = Sides + (i + 1) % Sides;
			const int32 D = Sides + i;
			Mesh.Loops.Add(FNRawMeshLoop(TArray<int32>{ A, B, C, D }));
		}
		Mesh.CalculateCenterAndBounds();
		Mesh.Validate();
		return Mesh;
	}
}

class FNRawMeshPerfTests
{
public:
	/** Validate on a 12-tri / 8-vert cube — the cheapest realistic case. Exercises CheckConvex+CheckNonTris+CheckBounds. */
	static void Validate_Box()
	{
		FNRawMesh Mesh = NEXUS::PerfTests::NCore::FNRawMeshHarness::MakeChaosBox(10.0);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshPerfTests_Validate_Box,
				NEXUS::PerfTests::NCore::FNRawMeshHarness::SmallMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshHarness::SmallIterations; ++i)
			{
				Mesh.Validate();
			}
			NTestTimer.ManualStop();
		}
	}

	/** Validate on a ~224-tri sphere stand-in for a typical cell hull. CheckConvex is O(V * F) on this. */
	static void Validate_HullMedium()
	{
		FNRawMesh Mesh = NEXUS::PerfTests::NCore::FNRawMeshHarness::MakeSphere(50.0, 16, 8);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshPerfTests_Validate_HullMedium,
				NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumIterations; ++i)
			{
				Mesh.Validate();
			}
			NTestTimer.ManualStop();
		}
	}

	/** Validate on a ~960-tri / ~482-vert sphere stand-in for a dense world collision mesh. */
	static void Validate_HullLarge()
	{
		FNRawMesh Mesh = NEXUS::PerfTests::NCore::FNRawMeshHarness::MakeSphere(50.0, 32, 16);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshPerfTests_Validate_HullLarge,
				NEXUS::PerfTests::NCore::FNRawMeshHarness::LargeMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshHarness::LargeIterations; ++i)
			{
				Mesh.Validate();
			}
			NTestTimer.ManualStop();
		}
	}

	/** CalculateFaceLoops on a triangulated medium sphere — exercises union-find + boundary walk. */
	static void CalculateFaceLoops_HullMedium()
	{
		FNRawMesh Mesh = NEXUS::PerfTests::NCore::FNRawMeshHarness::MakeSphere(50.0, 16, 8);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshPerfTests_CalculateFaceLoops_HullMedium,
				NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumIterations; ++i)
			{
				Mesh.CalculateFaceLoops();
			}
			NTestTimer.ManualStop();
		}
	}

	/** CalculateFaceLoops on a triangulated dense sphere. */
	static void CalculateFaceLoops_HullLarge()
	{
		FNRawMesh Mesh = NEXUS::PerfTests::NCore::FNRawMeshHarness::MakeSphere(50.0, 32, 16);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshPerfTests_CalculateFaceLoops_HullLarge,
				NEXUS::PerfTests::NCore::FNRawMeshHarness::LargeMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshHarness::LargeIterations; ++i)
			{
				Mesh.CalculateFaceLoops();
			}
			NTestTimer.ManualStop();
		}
	}

	/** ConvertToTriangles on an n-gon prism — exercises the convex fan-triangulation path on real n-gon faces. */
	static void ConvertToTriangles_NgonFanPath()
	{
		// Pre-build N independent copies so each iteration triangulates from scratch (ConvertToTriangles
		// early-outs once the input is already triangulated).
		const int32 Count = NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumIterations;
		TArray<FNRawMesh> Meshes;
		Meshes.Reserve(Count);
		for (int32 i = 0; i < Count; ++i)
		{
			Meshes.Add(NEXUS::PerfTests::NCore::FNRawMeshHarness::MakeNgonPrism(8, 50.0, 100.0));
		}

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshPerfTests_ConvertToTriangles_NgonFanPath,
				NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumMaxDuration)
			for (int32 i = 0; i < Count; ++i)
			{
				Meshes[i].ConvertToTriangles();
			}
			NTestTimer.ManualStop();
		}
	}

	/** SplitEdge baseline on a 12-tri box, single split per iteration on a freshly-prepared mesh. */
	static void SplitEdge_Box()
	{
		const int32 Count = NEXUS::PerfTests::NCore::FNRawMeshHarness::SmallIterations;
		TArray<FNRawMesh> Meshes;
		Meshes.Reserve(Count);
		for (int32 i = 0; i < Count; ++i)
		{
			Meshes.Add(NEXUS::PerfTests::NCore::FNRawMeshHarness::MakeChaosBox(10.0));
		}

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshPerfTests_SplitEdge_Box,
				NEXUS::PerfTests::NCore::FNRawMeshHarness::SmallMaxDuration)
			for (int32 i = 0; i < Count; ++i)
			{
				Meshes[i].SplitEdge(0, 1);
			}
			NTestTimer.ManualStop();
		}
	}

	/** CreateDynamicMesh on a medium hull — captures the per-vertex / per-tri Insert dance into FDynamicMesh3. */
	static void CreateDynamicMesh_HullMedium()
	{
		FNRawMesh Mesh = NEXUS::PerfTests::NCore::FNRawMeshHarness::MakeSphere(50.0, 16, 8);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshPerfTests_CreateDynamicMesh_HullMedium,
				NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumIterations; ++i)
			{
				FDynamicMesh3 Dyn = Mesh.CreateDynamicMesh(false);
			}
			NTestTimer.ManualStop();
		}
	}

	/** GetEdgeIndices baseline — TSet-backed undirected edge dedup over a medium hull. */
	static void GetEdgeIndices_HullMedium()
	{
		FNRawMesh Mesh = NEXUS::PerfTests::NCore::FNRawMeshHarness::MakeSphere(50.0, 16, 8);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshPerfTests_GetEdgeIndices_HullMedium,
				NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumIterations; ++i)
			{
				TArray<FIntVector2> Edges = Mesh.GetEdgeIndices();
			}
			NTestTimer.ManualStop();
		}
	}

	/** CalculateCenterAndBounds on a dense mesh — pure linear-in-vertices reduction baseline. */
	static void CalculateCenterAndBounds_HullLarge()
	{
		FNRawMesh Mesh = NEXUS::PerfTests::NCore::FNRawMeshHarness::MakeSphere(50.0, 32, 16);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshPerfTests_CalculateCenterAndBounds_HullLarge,
				NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumIterations; ++i)
			{
				Mesh.CalculateCenterAndBounds();
			}
			NTestTimer.ManualStop();
		}
	}

	/** RotatedAroundPivot on a dense mesh — per-vertex quaternion rotate + bounds rebuild. */
	static void RotatedAroundPivot_HullLarge()
	{
		FNRawMesh Mesh = NEXUS::PerfTests::NCore::FNRawMeshHarness::MakeSphere(50.0, 32, 16);
		const FRotator Rot(15.0, 30.0, 45.0);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshPerfTests_RotatedAroundPivot_HullLarge,
				NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumIterations; ++i)
			{
				Mesh.RotatedAroundPivot(FVector::ZeroVector, Rot);
			}
			NTestTimer.ManualStop();
		}
	}

	/**
	 * Deep copy of a ~224-tri sphere hull whose face-plane cache is already warm — the isolated cost of the
	 * `Hull = InputData->CellDetails.Hull;` line in FNAssemblyGraphCellNode's constructor. Each iteration copies
	 * into a fresh (empty) destination so every Vertices / Loops / cached-plane array is allocated from scratch,
	 * mirroring the per-candidate `new` node the graph builder churns through. The destinations escape into a
	 * pre-sized array so the compiler cannot elide the member copies it never reads back.
	 */
	static void CopyConstruct_HullMedium()
	{
		FNRawMesh Source = NEXUS::PerfTests::NCore::FNRawMeshHarness::MakeSphere(50.0, 16, 8);
		// Warm the source cache so the copy actually pays to duplicate the cached face-plane arrays.
		Source.EnsureCachedFacePlanes();

		TArray<FNRawMesh> Dest;
		Dest.SetNum(NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumIterations);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshPerfTests_CopyConstruct_HullMedium,
				NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumIterations; ++i)
			{
				Dest[i] = Source;
			}
			NTestTimer.ManualStop();
		}
	}

	/** Same isolated copy cost as CopyConstruct_HullMedium but for a 12-tri Chaos box — the most common cell hull. */
	static void CopyConstruct_Box()
	{
		FNRawMesh Source = NEXUS::PerfTests::NCore::FNRawMeshHarness::MakeChaosBox(10.0);
		Source.EnsureCachedFacePlanes();

		TArray<FNRawMesh> Dest;
		Dest.SetNum(NEXUS::PerfTests::NCore::FNRawMeshHarness::SmallIterations);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshPerfTests_CopyConstruct_Box,
				NEXUS::PerfTests::NCore::FNRawMeshHarness::SmallMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshHarness::SmallIterations; ++i)
			{
				Dest[i] = Source;
			}
			NTestTimer.ManualStop();
		}
	}

	/**
	 * Full FNAssemblyGraphCellNode hull-bake pattern on a ~224-tri sphere: copy the template hull, bake a world
	 * transform into it, then re-cache the face planes for the convex fast path. This is the realistic per-candidate
	 * cost the graph builder pays; the copy is one component, the transform + recache are the rest.
	 */
	static void BakeHull_HullMedium()
	{
		FNRawMesh Source = NEXUS::PerfTests::NCore::FNRawMeshHarness::MakeSphere(50.0, 16, 8);
		Source.EnsureCachedFacePlanes();
		const FTransform WorldTransform(FRotator(15.0, 30.0, 45.0), FVector(100.0, 200.0, 300.0));

		TArray<FNRawMesh> Dest;
		Dest.SetNum(NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumIterations);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRawMeshPerfTests_BakeHull_HullMedium,
				NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNRawMeshHarness::MediumIterations; ++i)
			{
				Dest[i] = Source;
				Dest[i].ApplyTransform(WorldTransform);
				if (Dest[i].IsConvex())
				{
					Dest[i].EnsureCachedFacePlanes();
				}
			}
			NTestTimer.ManualStop();
		}
	}
};

N_TEST_PERF(FNRawMeshPerfTests_Validate_Box,
	"NEXUS::PerfTests::NCore::FNRawMesh::Validate_Box",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshPerfTests::Validate_Box));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshPerfTests_Validate_HullMedium,
	"NEXUS::PerfTests::NCore::FNRawMesh::Validate_HullMedium",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshPerfTests::Validate_HullMedium));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshPerfTests_Validate_HullLarge,
	"NEXUS::PerfTests::NCore::FNRawMesh::Validate_HullLarge",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshPerfTests::Validate_HullLarge));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshPerfTests_CalculateFaceLoops_HullMedium,
	"NEXUS::PerfTests::NCore::FNRawMesh::CalculateFaceLoops_HullMedium",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshPerfTests::CalculateFaceLoops_HullMedium));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshPerfTests_CalculateFaceLoops_HullLarge,
	"NEXUS::PerfTests::NCore::FNRawMesh::CalculateFaceLoops_HullLarge",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshPerfTests::CalculateFaceLoops_HullLarge));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshPerfTests_ConvertToTriangles_NgonFanPath,
	"NEXUS::PerfTests::NCore::FNRawMesh::ConvertToTriangles_NgonFanPath",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshPerfTests::ConvertToTriangles_NgonFanPath));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshPerfTests_SplitEdge_Box,
	"NEXUS::PerfTests::NCore::FNRawMesh::SplitEdge_Box",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshPerfTests::SplitEdge_Box));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshPerfTests_CreateDynamicMesh_HullMedium,
	"NEXUS::PerfTests::NCore::FNRawMesh::CreateDynamicMesh_HullMedium",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshPerfTests::CreateDynamicMesh_HullMedium));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshPerfTests_GetEdgeIndices_HullMedium,
	"NEXUS::PerfTests::NCore::FNRawMesh::GetEdgeIndices_HullMedium",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshPerfTests::GetEdgeIndices_HullMedium));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshPerfTests_CalculateCenterAndBounds_HullLarge,
	"NEXUS::PerfTests::NCore::FNRawMesh::CalculateCenterAndBounds_HullLarge",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshPerfTests::CalculateCenterAndBounds_HullLarge));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshPerfTests_RotatedAroundPivot_HullLarge,
	"NEXUS::PerfTests::NCore::FNRawMesh::RotatedAroundPivot_HullLarge",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshPerfTests::RotatedAroundPivot_HullLarge));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshPerfTests_CopyConstruct_HullMedium,
	"NEXUS::PerfTests::NCore::FNRawMesh::CopyConstruct_HullMedium",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshPerfTests::CopyConstruct_HullMedium));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshPerfTests_CopyConstruct_Box,
	"NEXUS::PerfTests::NCore::FNRawMesh::CopyConstruct_Box",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshPerfTests::CopyConstruct_Box));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRawMeshPerfTests_BakeHull_HullMedium,
	"NEXUS::PerfTests::NCore::FNRawMesh::BakeHull_HullMedium",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRawMeshPerfTests::BakeHull_HullMedium));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

#endif //WITH_TESTS

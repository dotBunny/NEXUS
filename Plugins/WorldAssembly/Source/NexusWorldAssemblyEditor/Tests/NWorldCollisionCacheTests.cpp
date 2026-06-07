// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldCollisionCache.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TriggerVolume.h"
#include "Types/NRawMeshUtils.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionCacheHarness
{
	/** Engine cube spans 100uu (half-extent 50uu), so a unit scale maps to this half-extent. */
	constexpr double CubeHalfExtent = 50.0;

	/** Bounds generous enough to contain the test box's actor bounds. */
	static TArray<FBoxSphereBounds> WideBounds()
	{
		return { FBoxSphereBounds(FBox(FVector(-500.0), FVector(500.0))) };
	}

	/**
	 * Spawn an axis-aligned, world-static blocking box built from the engine cube mesh.
	 * @return The spawned actor, or nullptr when the cube mesh could not be loaded.
	 */
	static AStaticMeshActor* SpawnBox(UWorld* World, const FVector& Center, const FVector& HalfExtent)
	{
		UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (Cube == nullptr)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(Center, FRotator::ZeroRotator, SpawnParameters);
		if (Actor == nullptr)
		{
			return nullptr;
		}

		UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
		Mesh->SetMobility(EComponentMobility::Movable);
		Mesh->SetStaticMesh(Cube);
		Mesh->SetWorldScale3D(HalfExtent / CubeHalfExtent);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionObjectType(ECC_WorldStatic);
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
		Actor->SetActorEnableCollision(true);

		return Actor;
	}
}

N_TEST_HIGH(FNWorldCollisionCacheTests_Build_GathersBlockingActor,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionCache::Build::GathersBlockingActor",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies a collision-enabled static mesh inside the bounds contributes geometry that a contained point penetrates.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionCacheHarness;
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		if (SpawnBox(World, FVector::ZeroVector, FVector(50.0)) == nullptr)
		{
			ADD_ERROR("Could not spawn the test box (engine cube mesh unavailable).");
			return;
		}

		const FNRawMesh Mesh = FNWorldCollisionCache::Build(World, WideBounds());
		CHECK_MESSAGE(TEXT("A blocking actor in bounds should contribute geometry."), Mesh.Loops.Num() > 0)

		// A point at the box center should report penetration; a point well outside should not.
		const float InsideDepth = FNRawMeshUtils::GetIntersectDepth(Mesh, FVector::ZeroVector, FRotator::ZeroRotator, FVector::ZeroVector);
		CHECK_MESSAGE(TEXT("A point inside the gathered geometry should report positive penetration."), InsideDepth > 0.f)

		const float OutsideDepth = FNRawMeshUtils::GetIntersectDepth(Mesh, FVector::ZeroVector, FRotator::ZeroRotator, FVector(0, 0, 400));
		CHECK_MESSAGE(TEXT("A point clear of the geometry should report no penetration."), OutsideDepth <= 0.f)
	});
}

N_TEST_MEDIUM(FNWorldCollisionCacheTests_Build_ExcludesVolumes,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionCache::Build::ExcludesVolumes",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies the world-actor filter drops AVolumes (generation inputs, not obstacles).
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionCacheHarness;
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		// AVolume is abstract; ATriggerVolume is a concrete subclass that still satisfies the IsA<AVolume> filter.
		World->SpawnActor<ATriggerVolume>(FVector::ZeroVector, FRotator::ZeroRotator);

		const FNRawMesh Mesh = FNWorldCollisionCache::Build(World, WideBounds());
		CHECK_MESSAGE(TEXT("A lone volume should contribute no world-collision geometry."), Mesh.Loops.Num() == 0)
	});
}

N_TEST_MEDIUM(FNWorldCollisionCacheTests_Build_NullWorldReturnsEmpty,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionCache::Build::NullWorldReturnsEmpty",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies a null world is handled cleanly with an empty mesh.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionCacheHarness;
	const FNRawMesh Mesh = FNWorldCollisionCache::Build(nullptr, WideBounds());
	CHECK_MESSAGE(TEXT("A null world should yield an empty mesh."), Mesh.Loops.Num() == 0)
}

#endif //WITH_TESTS

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldCollisionPreview.h"
#include "Developer/NTestUtils.h"

#include "Engine/World.h"
#include "Macros/NTestMacros.h"
#include "NWorldCollisionBaker.h"
#include "NWorldCollisionCacheActor.h"
#include "NWorldAssemblySettings.h"
#include "Organ/NOrganComponent.h"
#include "Organ/NOrganVolume.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreviewHarness
{
	/** @return An organ volume covering a large region, so world geometry placed at the origin falls inside it. */
	static UNOrganComponent* SpawnCoveringOrgan(UWorld* World)
	{
		ANOrganVolume* Volume = World->SpawnActor<ANOrganVolume>(ANOrganVolume::StaticClass(),
			FVector::ZeroVector, FRotator::ZeroRotator);
		if (Volume == nullptr) return nullptr;

		Volume->SetActorScale3D(FVector(100.0));
		return Volume->GetOrganComponent();
	}
}

N_TEST_CRITICAL(FNWorldCollisionPreviewTests_GetState_UnbakedWorldReportsNotBaked,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreview::GetState::UnbakedWorldReportsNotBaked",
	N_TEST_CONTEXT_EDITOR)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreviewHarness;

	// The case that replaced the old live gather. A level nobody has baked must say so rather than quietly reporting
	// an empty world, which downstream reads as "there is no collision here".
	FNTestUtils::WorldTest(EWorldType::Editor, [](UWorld* World)
	{
		SpawnCoveringOrgan(World);
		FNWorldCollisionPreview::Invalidate(World);

		CHECK_MESSAGE(TEXT("A world with no cache actor must report NotBaked."),
			FNWorldCollisionPreview::GetState(World) == FNWorldCollisionPreview::EState::NotBaked);
		CHECK_FALSE_MESSAGE(TEXT("A NotBaked world must not report as available."),
			FNWorldCollisionPreview::IsAvailable(World));
		CHECK_EQUALS("A NotBaked world must yield an empty mesh.",
			FNWorldCollisionPreview::GetMesh(World).Loops.Num(), 0);
	});
}

N_TEST_MEDIUM(FNWorldCollisionPreviewTests_GetState_NullWorldReportsNotBaked,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreview::GetState::NullWorldReportsNotBaked",
	N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("A null world must report NotBaked rather than asserting."),
		FNWorldCollisionPreview::GetState(nullptr) == FNWorldCollisionPreview::EState::NotBaked);
	CHECK_EQUALS("A null world must yield an empty mesh.", FNWorldCollisionPreview::GetMesh(nullptr).Loops.Num(), 0);
	CHECK_MESSAGE(TEXT("A null world must yield an empty BVH."), FNWorldCollisionPreview::GetBVH(nullptr).IsEmpty());
}

N_TEST_CRITICAL(FNWorldCollisionPreviewTests_GetMesh_ShowsBakedGeometry,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreview::GetMesh::ShowsBakedGeometry",
	N_TEST_CONTEXT_EDITOR)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreviewHarness;

	// The whole point of the replacement: what is previewed comes from the bake, so it is the same geometry an
	// assembly collides against rather than a second derivation of it.
	FNTestUtils::WorldTest(EWorldType::Editor, [](UWorld* World)
	{
		if (SpawnCoveringOrgan(World) == nullptr)
		{
			ADD_ERROR("Could not spawn an organ volume to bake against.");
			return;
		}

		FNWorldCollisionBaker::BakeWorld(World, UNWorldAssemblySettings::Get()->WorldCollisionSettings, true);
		FNWorldCollisionPreview::Invalidate(World);

		// A bake that found no collision geometry still produces a valid, current cache. The state has to reflect
		// that the level *was baked*, not that the bake happened to find nothing — an early blockout with organs and
		// no geometry is exactly this, and reading it as unbaked would nag forever however often it was baked.
		CHECK_MESSAGE(TEXT("A freshly baked world must report Available even when it holds no collision geometry."),
			FNWorldCollisionPreview::GetState(World) == FNWorldCollisionPreview::EState::Available);
		CHECK_MESSAGE(TEXT("A baked empty world must report as available."),
			FNWorldCollisionPreview::IsAvailable(World));
	});
}

N_TEST_HIGH(FNWorldCollisionPreviewTests_Invalidate_ForcesRebuildAndBumpsGeneration,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreview::Invalidate::ForcesRebuildAndBumpsGeneration",
	N_TEST_CONTEXT_EDITOR)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreviewHarness;

	// The generation is what the bone readout memoizes against, so it has to move whenever the preview could differ.
	FNTestUtils::WorldTest(EWorldType::Editor, [](UWorld* World)
	{
		SpawnCoveringOrgan(World);

		FNWorldCollisionPreview::GetState(World);
		const uint32 Before = FNWorldCollisionPreview::GetGeneration(World);

		CHECK_EQUALS("Repeated queries on an unchanged world must not move the generation.",
			static_cast<int32>(FNWorldCollisionPreview::GetGeneration(World)), static_cast<int32>(Before));

		FNWorldCollisionPreview::Invalidate(World);
		CHECK_MESSAGE(TEXT("Invalidate must bump the generation."),
			FNWorldCollisionPreview::GetGeneration(World) != Before);
	});
}

N_TEST_HIGH(FNWorldCollisionPreviewTests_Invalidate_NullInvalidatesEveryWorld,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreview::Invalidate::NullInvalidatesEveryWorld",
	N_TEST_CONTEXT_EDITOR)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreviewHarness;

	// Undo/redo takes this path: it can move geometry in ways not tied to any one reported world, so nothing is
	// assumed to still be current afterwards.
	FNTestUtils::WorldTest(EWorldType::Editor, [](UWorld* World)
	{
		SpawnCoveringOrgan(World);
		FNWorldCollisionPreview::GetState(World);
		const uint32 Before = FNWorldCollisionPreview::GetGeneration(World);

		FNWorldCollisionPreview::Invalidate(nullptr);

		CHECK_MESSAGE(TEXT("A world-agnostic invalidate must bump every tracked world's generation."),
			FNWorldCollisionPreview::GetGeneration(World) != Before);
	});
}

N_TEST_HIGH(FNWorldCollisionPreviewTests_GetState_WorldWithNoOrgansReportsNotBaked,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreview::GetState::WorldWithNoOrgansReportsNotBaked",
	N_TEST_CONTEXT_EDITOR)
{
	// A cell level is the real instance of this: no organs, so nothing addresses the pool and there is nothing to
	// preview. It must read as NotBaked rather than as an empty-but-valid answer.
	FNTestUtils::WorldTest(EWorldType::Editor, [](UWorld* World)
	{
		FNWorldCollisionPreview::Invalidate(World);

		CHECK_MESSAGE(TEXT("A world with no organs must report NotBaked."),
			FNWorldCollisionPreview::GetState(World) == FNWorldCollisionPreview::EState::NotBaked);
	});
}

N_TEST_HIGH(FNWorldCollisionPreviewTests_ShouldNotify_AnnouncesOncePerTransition,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreview::ShouldNotify::AnnouncesOncePerTransition",
	N_TEST_CONTEXT_EDITOR)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreviewHarness;

	// What keeps a standing condition from being reported as a stream of events. The visualizer asks after every
	// completed edit and the bone readout asks on every viewport redraw, so all but the first of those are the same
	// answer to the same question — and answering them all is what put a notification on screen every ten seconds.
	FNTestUtils::WorldTest(EWorldType::Editor, [](UWorld* World)
	{
		SpawnCoveringOrgan(World);
		FNWorldCollisionPreview::Invalidate(World);

		CHECK_MESSAGE(TEXT("The first observation of an unavailable state must notify."),
			FNWorldCollisionPreview::ShouldNotify(World, FNWorldCollisionPreview::EState::NotBaked));
		CHECK_FALSE_MESSAGE(TEXT("Re-observing the same state must not notify again."),
			FNWorldCollisionPreview::ShouldNotify(World, FNWorldCollisionPreview::EState::NotBaked));

		// Invalidation runs on every move of every collision-relevant actor. It drops the memo; it must not re-arm
		// the notification, or this is back to speaking once per edit.
		FNWorldCollisionPreview::Invalidate(World);
		CHECK_FALSE_MESSAGE(TEXT("Invalidating must not re-arm the notification."),
			FNWorldCollisionPreview::ShouldNotify(World, FNWorldCollisionPreview::EState::NotBaked));

		// A different unavailable state is a different thing to say, so it gets said.
		CHECK_MESSAGE(TEXT("Moving to a different unavailable state must notify."),
			FNWorldCollisionPreview::ShouldNotify(World, FNWorldCollisionPreview::EState::Stale));
	});
}

N_TEST_MEDIUM(FNWorldCollisionPreviewTests_ShouldNotify_SaysNothingWhenThereIsNothingToSay,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreview::ShouldNotify::SaysNothingWhenThereIsNothingToSay",
	N_TEST_CONTEXT_EDITOR)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreviewHarness;

	CHECK_FALSE_MESSAGE(TEXT("A null world must never notify."),
		FNWorldCollisionPreview::ShouldNotify(nullptr, FNWorldCollisionPreview::EState::Stale));

	FNTestUtils::WorldTest(EWorldType::Editor, [](UWorld* World)
	{
		SpawnCoveringOrgan(World);

		CHECK_FALSE_MESSAGE(TEXT("An available world has no complaint to make."),
			FNWorldCollisionPreview::ShouldNotify(World, FNWorldCollisionPreview::EState::Available));
	});
}

N_TEST_HIGH(FNWorldCollisionPreviewTests_ShouldNotify_BakingReArmsTheNotification,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreview::ShouldNotify::BakingReArmsTheNotification",
	N_TEST_CONTEXT_EDITOR)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldCollisionPreviewHarness;

	// The other half of the edge trigger. Having said it once, the preview still has to be able to say it again once
	// the user has acted — otherwise a level would be told about its collision exactly once per editor session. A
	// bake is the only thing that resolves the condition, so it is what re-arms the notification, through the OnBaked
	// subscription the module registers.
	FNTestUtils::WorldTest(EWorldType::Editor, [](UWorld* World)
	{
		if (SpawnCoveringOrgan(World) == nullptr)
		{
			ADD_ERROR("Could not spawn an organ volume to bake against.");
			return;
		}

		FNWorldCollisionPreview::Invalidate(World);

		CHECK_MESSAGE(TEXT("The first staleness must be announced."),
			FNWorldCollisionPreview::ShouldNotify(World, FNWorldCollisionPreview::EState::Stale));
		CHECK_FALSE_MESSAGE(TEXT("The same staleness must not be announced twice."),
			FNWorldCollisionPreview::ShouldNotify(World, FNWorldCollisionPreview::EState::Stale));

		FNWorldCollisionBaker::BakeWorld(World, UNWorldAssemblySettings::Get()->WorldCollisionSettings, true);

		CHECK_MESSAGE(TEXT("Staleness found after a bake must be announced again."),
			FNWorldCollisionPreview::ShouldNotify(World, FNWorldCollisionPreview::EState::Stale));
	});
}

#endif //WITH_TESTS

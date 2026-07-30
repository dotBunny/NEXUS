// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NKillZoneActor.h"
#include "NKillZoneComponent.h"
#include "Developer/NDebugActor.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(UNKillZoneComponentTests_OnOverlapBegin_NullSubsystem_ReturnToActorPoolBehavior,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::OnOverlapBegin::NullSubsystem::ReturnToActorPoolBehavior",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that when the cached ActorPoolSubsystem is null and UnknownBehaviour is
	// ReturnToActorPool, OnOverlapBegin logs a warning and does not increment KillCount
	// rather than dereferencing the null subsystem pointer.
	AddExpectedMessage(TEXT("Unable to return the unknown"), ELogVerbosity::Warning);

	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		const FVector RemoteLocation(200.0f, 200.0f, 200.0f);
		ANKillZoneActor* KillZoneActor = World->SpawnActor<ANKillZoneActor>(RemoteLocation, FRotator::ZeroRotator);
		if (!KillZoneActor)
		{
			ADD_ERROR("Could not spawn ANKillZoneActor.");
			return;
		}

		UNKillZoneComponent* KillZone = Cast<UNKillZoneComponent>(KillZoneActor->GetRootComponent());
		if (!KillZone)
		{
			ADD_ERROR("Root component is not a UNKillZoneComponent.");
			return;
		}

		KillZone->ActorPoolSubsystem = nullptr;
		KillZone->UnknownBehaviour = ENKillZoneBehavior::ReturnToActorPool;

		ANDebugActor* UnknownActor = World->SpawnActor<ANDebugActor>();
		if (!UnknownActor)
		{
			ADD_ERROR("Could not spawn unknown test actor.");
			return;
		}

		KillZone->OnOverlapBegin(KillZone, UnknownActor, nullptr, 0, false, FHitResult());
		CHECK_EQUALS("Kill count must remain 0 when ReturnToActorPool runs against a null subsystem.", KillZone->GetKillCount(), 0)
	}, true);
}

N_TEST_HIGH(UNKillZoneComponentTests_OnOverlapBegin_NullSubsystem_KnownPoolPathSkipped,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::OnOverlapBegin::NullSubsystem::KnownPoolPathSkipped",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that with a null ActorPoolSubsystem the HasActorPool fast-path is short-circuited
	// and the kill zone falls through to UnknownBehaviour — here Ignore, leaving KillCount at 0.
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		const FVector RemoteLocation(200.0f, 200.0f, 200.0f);
		ANKillZoneActor* KillZoneActor = World->SpawnActor<ANKillZoneActor>(RemoteLocation, FRotator::ZeroRotator);
		if (!KillZoneActor)
		{
			ADD_ERROR("Could not spawn ANKillZoneActor.");
			return;
		}

		UNKillZoneComponent* KillZone = Cast<UNKillZoneComponent>(KillZoneActor->GetRootComponent());
		if (!KillZone)
		{
			ADD_ERROR("Root component is not a UNKillZoneComponent.");
			return;
		}

		KillZone->ActorPoolSubsystem = nullptr;
		KillZone->UnknownBehaviour = ENKillZoneBehavior::Ignore;

		ANDebugActor* UnknownActor = World->SpawnActor<ANDebugActor>();
		if (!UnknownActor)
		{
			ADD_ERROR("Could not spawn unknown test actor.");
			return;
		}

		KillZone->OnOverlapBegin(KillZone, UnknownActor, nullptr, 0, false, FHitResult());
		CHECK_EQUALS("Kill count must remain 0 when the known-pool path is short-circuited by a null subsystem.", KillZone->GetKillCount(), 0)
	}, true);
}

N_TEST_HIGH(UNKillZoneComponentTests_OnOverlapBegin_NullSubsystem_FellOutOfWorldStillWorks,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::OnOverlapBegin::NullSubsystem::FellOutOfWorldStillWorks",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that ApplyFellOutOfWorld does not depend on the cached subsystem and still
	// increments KillCount when ActorPoolSubsystem is null.
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		const FVector RemoteLocation(200.0f, 200.0f, 200.0f);
		ANKillZoneActor* KillZoneActor = World->SpawnActor<ANKillZoneActor>(RemoteLocation, FRotator::ZeroRotator);
		if (!KillZoneActor)
		{
			ADD_ERROR("Could not spawn ANKillZoneActor.");
			return;
		}

		UNKillZoneComponent* KillZone = Cast<UNKillZoneComponent>(KillZoneActor->GetRootComponent());
		if (!KillZone)
		{
			ADD_ERROR("Root component is not a UNKillZoneComponent.");
			return;
		}

		KillZone->ActorPoolSubsystem = nullptr;
		KillZone->UnknownBehaviour = ENKillZoneBehavior::ApplyFellOutOfWorld;

		ANDebugActor* UnknownActor = World->SpawnActor<ANDebugActor>();
		if (!UnknownActor)
		{
			ADD_ERROR("Could not spawn unknown test actor.");
			return;
		}

		KillZone->OnOverlapBegin(KillZone, UnknownActor, nullptr, 0, false, FHitResult());
		CHECK_EQUALS("Kill count should be 1 after FellOutOfWorld runs without a subsystem.", KillZone->GetKillCount(), 1)
	}, true);
}

#endif //WITH_TESTS

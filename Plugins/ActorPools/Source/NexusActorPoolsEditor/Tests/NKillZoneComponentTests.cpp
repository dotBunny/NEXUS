// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NActorPool.h"
#include "NActorPoolSubsystem.h"
#include "NKillZoneActor.h"
#include "NKillZoneComponent.h"
#include "NTestPooledActor.h"
#include "Developer/NDebugActor.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_CRITICAL(UNKillZoneComponentTests_KillCount_InitialStateIsZero,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::KillCount::InitialStateIsZero",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that a freshly spawned kill zone component starts with a kill count of zero.
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		ANKillZoneActor* KillZoneActor = World->SpawnActor<ANKillZoneActor>();
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

		CHECK_EQUALS("Kill count should be 0 after spawn.", KillZone->GetKillCount(), 0)
	});
}

N_TEST_HIGH(UNKillZoneComponentTests_KillCount_SetAndGet,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::KillCount::SetAndGet",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that SetKillCount persists and GetKillCount returns the assigned value.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		ANKillZoneActor* KillZoneActor = World->SpawnActor<ANKillZoneActor>();
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

		KillZone->SetKillCount(7);
		CHECK_EQUALS("GetKillCount should return the value passed to SetKillCount.", KillZone->GetKillCount(), 7)
	});
}

N_TEST_HIGH(UNKillZoneComponentTests_KillCount_Reset,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::KillCount::Reset",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that ResetKillCount sets the kill count back to zero regardless of the current value.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		ANKillZoneActor* KillZoneActor = World->SpawnActor<ANKillZoneActor>();
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

		KillZone->SetKillCount(12);
		KillZone->ResetKillCount();
		CHECK_EQUALS("ResetKillCount should return the kill count to zero.", KillZone->GetKillCount(), 0)
	});
}

N_TEST_HIGH(UNKillZoneComponentTests_OnOverlapBegin_IgnoresStaticActor,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::OnOverlapBegin::IgnoresStaticActor",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that an actor with a non-movable root component is silently ignored when
	// bIgnoreStaticActors is true (the default), leaving the kill count unchanged.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		// Need to spawn the zone away from origin so it doesnt trigger before actor settings are applied.
		FVector RemoteLocation = FVector(200.f,200.f,200.f);
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

		ANDebugActor* StaticActor = World->SpawnActor<ANDebugActor>();
		if (!StaticActor)
		{
			ADD_ERROR("Could not spawn static test actor.");
			return;
		}

		// Mark the overlapping actor's root as static so IsRootComponentMovable() returns false.
		if (StaticActor->GetRootComponent())
		{
			StaticActor->GetRootComponent()->SetMobility(EComponentMobility::Static);
		}

		KillZone->OnOverlapBegin(KillZone, StaticActor, nullptr, 0, false, FHitResult());
		CHECK_EQUALS("Kill count must remain 0 when ignoring a static actor.", KillZone->GetKillCount(), 0)
	});
}


N_TEST_CRITICAL(UNKillZoneComponentTests_OnOverlapBegin_IncrementOnInterfacedActorReturn,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::OnOverlapBegin::IncrementOnInterfacedActorReturn",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that overlapping a pooled INActorPoolItem actor returns it to its pool
	// and increments the kill count.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		// Need to spawn the zone away from origin so it doesnt trigger before actor settings are applied.
		FVector RemoteLocation = FVector(200.f,200.f,200.f);
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

		FNActorPoolSettings PoolSettings;
		PoolSettings.MinimumActorCount = 1;
		PoolSettings.MaximumActorCount = 1;
		PoolSettings.Strategy = ENActorPoolStrategy::Fixed;
		PoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);

		FNActorPool Pool = FNActorPool(World, ANTestPooledActor::StaticClass(), PoolSettings);
		Pool.Fill();

		ANTestPooledActor* PooledActor = Cast<ANTestPooledActor>(Pool.Spawn(FVector::ZeroVector, FRotator::ZeroRotator));
		if (!PooledActor)
		{
			ADD_ERROR("Pool.Spawn() returned nullptr unexpectedly.");
			Pool.Clear(true);
			return;
		}

		KillZone->OnOverlapBegin(KillZone, PooledActor, nullptr, 0, false, FHitResult());
		CHECK_EQUALS("Kill count should be 1 after the pooled actor is returned via overlap.", KillZone->GetKillCount(), 1)

		Pool.Clear(true);
	});
}


N_TEST_CRITICAL(UNKillZoneComponentTests_OnOverlapBegin_IncrementOnActorWithKnownPool,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::OnOverlapBegin::IncrementOnActorWithKnownPool",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that overlapping an actor that has a registered subsystem pool (but does not
	// implement INActorPoolItem) returns it to that pool and increments the kill count.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		// Need to spawn the zone away from origin so it doesnt trigger before actor settings are applied.
		FVector RemoteLocation = FVector(200.f,200.f,200.f);
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

		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		FNActorPoolSettings PoolSettings;
		PoolSettings.MinimumActorCount = 2;
		PoolSettings.MaximumActorCount = 2;
		PoolSettings.Strategy = ENActorPoolStrategy::Fixed;
		PoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);
		Subsystem->CreateActorPool(ANDebugActor::StaticClass(), PoolSettings);

		ANDebugActor* Actor = Subsystem->SpawnActor<ANDebugActor>(ANDebugActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (!Actor)
		{
			ADD_ERROR("SpawnActor returned nullptr unexpectedly.");
			return;
		}

		KillZone->OnOverlapBegin(KillZone, Actor, nullptr, 0, false, FHitResult());
		CHECK_EQUALS("Kill count should be 1 after returning an actor with a known subsystem pool.", KillZone->GetKillCount(), 1)
	});
}

N_TEST_HIGH(UNKillZoneComponentTests_OnOverlapBegin_UnknownBehaviorIgnore,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::OnOverlapBegin::UnknownBehaviorIgnore",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that an actor with no pool and no INActorPoolItem interface is silently ignored
	// when UnknownBehaviour is set to ENKillZoneBehavior::Ignore, leaving the kill count unchanged.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		// Need to spawn the zone away from origin so it doesnt trigger before actor settings are applied.
		FVector RemoteLocation = FVector(200.f,200.f,200.f);
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

		KillZone->UnknownBehaviour = ENKillZoneBehavior::Ignore;

		ANDebugActor* UnknownActor = World->SpawnActor<ANDebugActor>();
		if (!UnknownActor)
		{
			ADD_ERROR("Could not spawn unknown test actor.");
			return;
		}

		KillZone->OnOverlapBegin(KillZone, UnknownActor, nullptr, 0, false, FHitResult());
		CHECK_EQUALS("Kill count must remain 0 when UnknownBehaviour is Ignore.", KillZone->GetKillCount(), 0)
	});
}

N_TEST_HIGH(UNKillZoneComponentTests_OnOverlapBegin_UnknownBehaviorApplyFellOutOfWorld,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::OnOverlapBegin::UnknownBehaviorApplyFellOutOfWorld",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that an actor with no pool and no INActorPoolItem interface has FellOutOfWorld
	// applied (the default UnknownBehaviour) and the kill count is incremented.
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		// Need to spawn the zone away from origin so it doesnt trigger before actor settings are applied.
		FVector RemoteLocation = FVector(200.f,200.f,200.f);
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

		CHECK_EQUALS("Default UnknownBehaviour should be ApplyFellOutOfWorld.",
			static_cast<uint8>(KillZone->UnknownBehaviour), static_cast<uint8>(ENKillZoneBehavior::ApplyFellOutOfWorld))

		ANDebugActor* UnknownActor = World->SpawnActor<ANDebugActor>();
		if (!UnknownActor)
		{
			ADD_ERROR("Could not spawn unknown test actor.");
			return;
		}

		KillZone->OnOverlapBegin(KillZone, UnknownActor, nullptr, 0, false, FHitResult());
		CHECK_EQUALS("Kill count should be 1 after FellOutOfWorld is applied to the unknown actor.", KillZone->GetKillCount(), 1)
	}, true);
}

N_TEST_MEDIUM(UNKillZoneComponentTests_OnOverlapBegin_UnknownBehaviorApplyFellOutOfWorld_RootedActor,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::OnOverlapBegin::UnknownBehaviorApplyFellOutOfWorld_RootedActor",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that a rooted actor with the default ApplyFellOutOfWorld behaviour still
	// increments the kill count (FellOutOfWorld is invoked unconditionally) and that no
	// pool is implicitly created for the unknown actor.
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		// Need to spawn the zone away from origin so it doesnt trigger before actor settings are applied.
		FVector RemoteLocation = FVector(200.f,200.f,200.f);
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

		ANDebugActor* RootedActor = World->SpawnActor<ANDebugActor>();
		if (!RootedActor)
		{
			ADD_ERROR("Could not spawn rooted test actor.");
			return;
		}

		RootedActor->AddToRoot();
		UNActorPoolSubsystem* System = UNActorPoolSubsystem::Get(World);
		
		AddExpectedMessage(TEXT("UNKillZoneComponent unable to apply FellOutOfWorld to rooted unknown AActor"), ELogVerbosity::Warning);
		KillZone->OnOverlapBegin(KillZone, RootedActor, nullptr, 0, false, FHitResult());
		CHECK_EQUALS("A pool should not be made for the unknown rooted actor.", System->GetAllPools().Num(), 0)
		CHECK_EQUALS("Kill count must be 0 after ApplyFellOutOfWorld runs on the rooted actor.", KillZone->GetKillCount(), 0)

		RootedActor->RemoveFromRoot();
	}, true);
}

N_TEST_HIGH(UNKillZoneComponentTests_OnOverlapBegin_UnknownBehaviorReturnToActorPool_NoPool,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::OnOverlapBegin::UnknownBehaviorReturnToActorPool_NoPool",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that when UnknownBehaviour is ReturnToActorPool and the subsystem cannot route
	// the unknown actor (its UnknownBehavior is forced to Destroy so ReturnActor returns false),
	// the kill zone logs a warning and does not increment its kill count.
	AddExpectedMessage(TEXT("Unable to return the unknown"), ELogVerbosity::Type::Warning);

	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		// Need to spawn the zone away from origin so it doesn't trigger before actor settings are applied.
		const FVector RemoteLocation = FVector(200.f,200.f,200.f);
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

		UNActorPoolSubsystem* Subsystem = UNActorPoolSubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNActorPoolSubsystem from PIE world.");
			return;
		}

		// Project default is CreateDefaultPool, which would auto-pool and return true; force
		// Destroy so ReturnActor returns false and the kill zone takes the warning path.
		Subsystem->SetUnknownBehavior(ENActorPoolUnknownBehavior::Destroy);
		KillZone->UnknownBehaviour = ENKillZoneBehavior::ReturnToActorPool;

		ANDebugActor* UnknownActor = World->SpawnActor<ANDebugActor>();
		if (!UnknownActor)
		{
			ADD_ERROR("Could not spawn unknown test actor.");
			return;
		}

		KillZone->OnOverlapBegin(KillZone, UnknownActor, nullptr, 0, false, FHitResult());
		CHECK_EQUALS("Kill count must remain 0 when ReturnToActorPool fails for an unknown class.", KillZone->GetKillCount(), 0)
	}, true);
}

#endif //WITH_TESTS

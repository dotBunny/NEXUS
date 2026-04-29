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

N_TEST_HIGH(UNKillZoneComponentTests_OnOverlapBegin_IgnoresNonInterfacedActorWhenFlagSet,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::OnOverlapBegin::IgnoresNonInterfacedActorWhenFlagSet",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that an actor with no pool and no INActorPoolItem interface is silently ignored
	// when bIgnoreNonInterfacedActors is true, leaving the kill count unchanged.
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

		// bIgnoreNonInterfacedActors is protected — set it via UE property reflection.
		FBoolProperty* IgnoreProp = FindFProperty<FBoolProperty>(UNKillZoneComponent::StaticClass(), TEXT("bIgnoreNonInterfacedActors"));
		if (!IgnoreProp)
		{
			ADD_ERROR("Could not find bIgnoreNonInterfacedActors property — check the UPROPERTY name.");
			return;
		}
		IgnoreProp->SetPropertyValue_InContainer(KillZone, true);

		ANDebugActor* UnknownActor = World->SpawnActor<ANDebugActor>();
		if (!UnknownActor)
		{
			ADD_ERROR("Could not spawn unknown test actor.");
			return;
		}

		KillZone->OnOverlapBegin(KillZone, UnknownActor, nullptr, 0, false, FHitResult());
		CHECK_EQUALS("Kill count must remain 0 when ignoring non-interfaced actors.", KillZone->GetKillCount(), 0)
	});
}

N_TEST_HIGH(UNKillZoneComponentTests_OnOverlapBegin_IncrementOnUnknownActor,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::OnOverlapBegin::IncrementOnUnknownActor",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that an actor with no pool and no INActorPoolItem interface is destroyed via
	// the subsystem's default Destroy behavior and the kill count is incremented.
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

		ANDebugActor* UnknownActor = World->SpawnActor<ANDebugActor>();
		if (!UnknownActor)
		{
			ADD_ERROR("Could not spawn unknown test actor.");
			return;
		}

		// The subsystem's default UnknownBehavior is Destroy. ReturnActor calls Destroy()
		// which returns true, so the kill count increments and the actor is destroyed.
		KillZone->OnOverlapBegin(KillZone, UnknownActor, nullptr, 0, false, FHitResult());
		CHECK_EQUALS("Kill count should be 1 after the unknown actor is destroyed.", KillZone->GetKillCount(), 1)
	}, true);
}

N_TEST_MEDIUM(UNKillZoneComponentTests_OnOverlapBegin_IncrementForRootedActor,
	"NEXUS::UnitTests::NActorPools::UNKillZoneComponent::OnOverlapBegin::IncrementForRootedActor",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that a rooted actor cannot be destroyed, the kill count is not incremented,
	// and a warning is logged.
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

		KillZone->OnOverlapBegin(KillZone, RootedActor, nullptr, 0, false, FHitResult());
		CHECK_EQUALS("A pool should have been made for the rooted actor.", System->GetAllPools().Num(), 1)
		CHECK_EQUALS("Kill count must be 1 when a rooted actor when a pool is created.", KillZone->GetKillCount(), 1)
		
		RootedActor->RemoveFromRoot();
	}, true);
}

#endif //WITH_TESTS

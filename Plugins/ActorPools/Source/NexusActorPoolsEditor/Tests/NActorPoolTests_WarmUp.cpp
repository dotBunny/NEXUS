// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NActorPool.h"
#include "NActorPoolSubsystem.h"
#include "Developer/NDebugActor.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

// Covers the time-sliced warm-up path: a pool created with CreateObjectsPerTick > 0 defers creation and
// fills MinimumActorCount actors across several ticks rather than synchronously. The Fixed strategy is used
// throughout because it never grows beyond MinimumActorCount on demand, so a ticked warm-up's actor count is
// exactly observable with no create-on-demand overshoot. Its one on-demand behavior is the self-heal in
// ApplyStrategy() -> WarmUpDeficit(): if a Fixed pool's warm-up Tick() is never delivered, the first Get/Spawn
// fills the remaining deficit so the pool can't be left permanently empty (see FixedSelfHealsWhenNeverTicked).
//
// Every pool drops the ServerOnly flag (kept out of Flags below): the PIE test world has no auth game mode,
// so a ServerOnly pool would resolve to a client stub whose Tick() is a no-op, defeating the test.

N_TEST_CRITICAL(FNActorPoolTests_WarmUp_FixedDeferUntilTicked,
	"NEXUS::UnitTests::NActorPools::FNActorPool::WarmUp::FixedDeferUntilTicked",
	N_TEST_CONTEXT_EDITOR)
{
	// A throttled pool must NOT fill during construction; it starts empty and registers the subsystem for
	// ticking so warm-up can proceed over subsequent frames.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNActorPoolSubsystem, World)

		FNActorPoolSettings Settings;
		Settings.Strategy = ENActorPoolStrategy::Fixed;
		Settings.MinimumActorCount = 6;
		Settings.MaximumActorCount = 6;
		Settings.CreateObjectsPerTick = 2;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction);
		Subsystem->CreateActorPool(ANDebugActor::StaticClass(), Settings);

		FNActorPool* Pool = Subsystem->GetActorPool(ANDebugActor::StaticClass());
		if (Pool == nullptr)
		{
			ADD_ERROR("Pool was not created.");
			return;
		}

		CHECK_EQUALS("A throttled pool must not synchronously fill during construction.", Pool->GetAvailableCount(), 0)
		CHECK_MESSAGE(TEXT("A throttled pool must register the subsystem for ticking so warm-up can proceed."),
			Subsystem->IsTickable())
	});
}

N_TEST_CRITICAL(FNActorPoolTests_WarmUp_TickFillsPerTickAndClamps,
	"NEXUS::UnitTests::NActorPools::FNActorPool::WarmUp::TickFillsPerTickAndClamps",
	N_TEST_CONTEXT_EDITOR)
{
	// Tick() creates CreateObjectsPerTick actors per call, clamps the final partial tick so it never
	// overshoots MinimumActorCount, returns true through the tick that reaches the minimum, and false
	// thereafter (the one-shot dropout signal the subsystem uses to unregister the pool).
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNActorPoolSubsystem, World)

		// Minimum 5 with a per-tick of 2 exercises the clamp: 2, 2, then 1 (the remaining deficit).
		FNActorPoolSettings Settings;
		Settings.Strategy = ENActorPoolStrategy::Fixed;
		Settings.MinimumActorCount = 5;
		Settings.MaximumActorCount = 5;
		Settings.CreateObjectsPerTick = 2;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction);
		Subsystem->CreateActorPool(ANDebugActor::StaticClass(), Settings);

		FNActorPool* Pool = Subsystem->GetActorPool(ANDebugActor::StaticClass());
		if (Pool == nullptr)
		{
			ADD_ERROR("Pool was not created.");
			return;
		}

		const bool bTick1 = Pool->Tick();
		CHECK_EQUALS("First tick creates CreateObjectsPerTick (2) actors.", Pool->GetAvailableCount(), 2)
		CHECK_MESSAGE(TEXT("Tick must report it is still warming after the first tick."), bTick1)

		const bool bTick2 = Pool->Tick();
		CHECK_EQUALS("Second tick adds another 2 actors.", Pool->GetAvailableCount(), 4)
		CHECK_MESSAGE(TEXT("Tick must report it is still warming after the second tick."), bTick2)

		const bool bTick3 = Pool->Tick();
		CHECK_EQUALS("Third tick clamps to the remaining deficit (1), reaching the minimum without overshoot.",
			Pool->GetAvailableCount(), 5)
		CHECK_MESSAGE(TEXT("The tick that reaches the minimum still reports warming; dropout happens on the next tick."),
			bTick3)

		const bool bTick4 = Pool->Tick();
		CHECK_EQUALS("A warmed pool creates no further actors.", Pool->GetAvailableCount(), 5)
		CHECK_FALSE_MESSAGE(
			TEXT("Once the minimum is reached the next Tick reports completion so the subsystem can unregister it."),
			bTick4)
	});
}

N_TEST_HIGH(FNActorPoolTests_WarmUp_DrivenBySubsystemTick,
	"NEXUS::UnitTests::NActorPools::FNActorPool::WarmUp::DrivenBySubsystemTick",
	N_TEST_CONTEXT_EDITOR)
{
	// Driving the subsystem tick warms a registered pool to its minimum over multiple frames and then
	// stops creating (no overshoot), proving the create-time tick registration is wired end to end.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNActorPoolSubsystem, World)

		FNActorPoolSettings Settings;
		Settings.Strategy = ENActorPoolStrategy::Fixed;
		Settings.MinimumActorCount = 4;
		Settings.MaximumActorCount = 4;
		Settings.CreateObjectsPerTick = 1;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction);
		Subsystem->CreateActorPool(ANDebugActor::StaticClass(), Settings);

		FNActorPool* Pool = Subsystem->GetActorPool(ANDebugActor::StaticClass());
		if (Pool == nullptr)
		{
			ADD_ERROR("Pool was not created.");
			return;
		}
		CHECK_EQUALS("Pool starts empty before any subsystem tick.", Pool->GetAvailableCount(), 0)

		// More ticks than strictly required; warm-up is a one-shot so the extra ticks must not overshoot.
		for (int32 i = 0; i < 8; ++i)
		{
			Subsystem->Tick(0.016f);
		}

		CHECK_EQUALS("Subsystem ticks warm the pool exactly to its minimum without overshoot.",
			Pool->GetAvailableCount(), 4)
	});
}

N_TEST_HIGH(FNActorPoolTests_WarmUp_FixedYieldsMinimumThenNull,
	"NEXUS::UnitTests::NActorPools::FNActorPool::WarmUp::FixedYieldsMinimumThenNull",
	N_TEST_CONTEXT_EDITOR)
{
	// A Fixed pool never grows on demand (ApplyStrategy returns false), so warm-up is the only source of
	// actors: once warmed it must hand out exactly MinimumActorCount actors and then return null.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNActorPoolSubsystem, World)

		FNActorPoolSettings Settings;
		Settings.Strategy = ENActorPoolStrategy::Fixed;
		Settings.MinimumActorCount = 3;
		Settings.MaximumActorCount = 3;
		Settings.CreateObjectsPerTick = 1;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction);
		Subsystem->CreateActorPool(ANDebugActor::StaticClass(), Settings);

		FNActorPool* Pool = Subsystem->GetActorPool(ANDebugActor::StaticClass());
		if (Pool == nullptr)
		{
			ADD_ERROR("Pool was not created.");
			return;
		}

		// Drive warm-up to completion; the iteration cap guards the test against a non-terminating Tick.
		for (int32 i = 0; i < 16 && Pool->Tick(); ++i) {}
		CHECK_EQUALS("Warm-up must populate the pool to its minimum.", Pool->GetAvailableCount(), 3)

		for (int32 i = 0; i < 3; ++i)
		{
			if (Pool->Spawn(FVector::ZeroVector, FRotator::ZeroRotator) == nullptr)
			{
				ADD_ERROR(FString::Printf(TEXT("Spawn %d should succeed from a warmed Fixed pool."), i));
			}
		}

		CHECK_MESSAGE(TEXT("A Fixed pool must return null once its warmed actors are exhausted; it never grows."),
			Pool->Spawn(FVector::ZeroVector, FRotator::ZeroRotator) == nullptr)
	});
}

N_TEST_MEDIUM(FNActorPoolTests_WarmUp_UpdateSettingsStopsTicking,
	"NEXUS::UnitTests::NActorPools::FNActorPool::WarmUp::UpdateSettingsStopsTicking",
	N_TEST_CONTEXT_EDITOR)
{
	// Re-applying settings with CreateObjectsPerTick <= 0 must unregister an in-progress warm-up so the
	// subsystem stops ticking the pool; its actor count then freezes mid-warm-up.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNActorPoolSubsystem, World)

		FNActorPoolSettings Settings;
		Settings.Strategy = ENActorPoolStrategy::Fixed;
		Settings.MinimumActorCount = 8;
		Settings.MaximumActorCount = 8;
		Settings.CreateObjectsPerTick = 2;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction);
		Subsystem->CreateActorPool(ANDebugActor::StaticClass(), Settings);

		FNActorPool* Pool = Subsystem->GetActorPool(ANDebugActor::StaticClass());
		if (Pool == nullptr)
		{
			ADD_ERROR("Pool was not created.");
			return;
		}

		// One frame of warm-up, then disable throttled creation entirely.
		Subsystem->Tick(0.016f);
		const int32 AvailableAfterOneTick = Pool->GetAvailableCount();
		CHECK_EQUALS("A single subsystem tick should create CreateObjectsPerTick (2) actors.",
			AvailableAfterOneTick, 2)

		FNActorPoolSettings Disabled = Settings;
		Disabled.CreateObjectsPerTick = 0;
		Pool->UpdateSettings(Disabled);

		for (int32 i = 0; i < 8; ++i)
		{
			Subsystem->Tick(0.016f);
		}

		CHECK_EQUALS("Once CreateObjectsPerTick is disabled the pool must be unregistered and stop creating actors.",
			Pool->GetAvailableCount(), AvailableAfterOneTick)
	});
}

N_TEST_HIGH(FNActorPoolTests_WarmUp_FixedSelfHealsWhenNeverTicked,
	"NEXUS::UnitTests::NActorPools::FNActorPool::WarmUp::FixedSelfHealsWhenNeverTicked",
	N_TEST_CONTEXT_EDITOR)
{
	// Regression: a Fixed pool configured for time-sliced warm-up (CreateObjectsPerTick > 0) whose warm-up
	// Tick() is never delivered — e.g. it was created in a world that won't tick again — must not be left
	// permanently empty. The first Spawn self-heals via ApplyStrategy() -> WarmUpDeficit(), filling the
	// remaining deficit to MinimumActorCount, after which it behaves like any warmed Fixed pool: it hands out
	// exactly MinimumActorCount actors and then returns null (the self-heal must not overshoot the minimum).
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNActorPoolSubsystem, World)

		FNActorPoolSettings Settings;
		Settings.Strategy = ENActorPoolStrategy::Fixed;
		Settings.MinimumActorCount = 4;
		Settings.MaximumActorCount = 4;
		Settings.CreateObjectsPerTick = 2;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction);
		Subsystem->CreateActorPool(ANDebugActor::StaticClass(), Settings);

		FNActorPool* Pool = Subsystem->GetActorPool(ANDebugActor::StaticClass());
		if (Pool == nullptr)
		{
			ADD_ERROR("Pool was not created.");
			return;
		}

		// Deliberately never ticked: the throttled pool defers creation and so starts empty. Without the
		// self-heal this is the permanently-dead state the regression guards against.
		CHECK_EQUALS("A throttled Fixed pool starts empty before any tick.", Pool->GetAvailableCount(), 0)

		// First Spawn on the never-ticked pool must self-heal rather than return null.
		if (Pool->Spawn(FVector::ZeroVector, FRotator::ZeroRotator) == nullptr)
		{
			ADD_ERROR("First Spawn on a never-ticked Fixed pool must self-heal rather than return null.");
			return;
		}

		// The self-heal creates the full minimum (4) in one shot; one handed out leaves the remainder available.
		CHECK_EQUALS("Self-heal fills to MinimumActorCount; one handed out leaves the rest available.",
			Pool->GetAvailableCount(), 3)

		// The remaining three also come from the pool: total handed out equals MinimumActorCount.
		for (int32 i = 1; i < 4; ++i)
		{
			if (Pool->Spawn(FVector::ZeroVector, FRotator::ZeroRotator) == nullptr)
			{
				ADD_ERROR(FString::Printf(TEXT("Spawn %d should succeed from the self-healed Fixed pool."), i));
			}
		}

		// Self-heal must not overshoot: a Fixed pool still caps at MinimumActorCount and returns null once exhausted.
		CHECK_MESSAGE(TEXT("A self-healed Fixed pool still caps at MinimumActorCount, returning null once exhausted."),
			Pool->Spawn(FVector::ZeroVector, FRotator::ZeroRotator) == nullptr)
	});
}

#endif //WITH_TESTS

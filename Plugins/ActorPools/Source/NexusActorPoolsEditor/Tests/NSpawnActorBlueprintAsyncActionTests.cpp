// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NActorPool.h"
#include "NActorPoolSubsystem.h"
#include "NSpawnActorBlueprintAsyncAction.h"
#include "NTestPooledActor.h"
#include "Developer/NDebugActor.h"
#include "Developer/NTestUtils.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(UNSpawnActorBlueprintAsyncActionTests_OnHasPool_NullPool,
	"NEXUS::UnitTests::NActorPools::UNSpawnActorBlueprintAsyncAction::OnHasPool::NullPool",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that the defensive guard at the top of OnHasPool short-circuits on a null pool —
	// the OnCreatedPoolHandle reset and Completed broadcast that follow must not run.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNActorPoolSubsystem, World)

		UNSpawnActorBlueprintAsyncAction* Action = NewObject<UNSpawnActorBlueprintAsyncAction>();
		Action->WorldContext = World;
		Action->ActorClass = TSoftClassPtr<AActor>(ANTestPooledActor::StaticClass());

		Action->OnCreatedPoolHandle = Subsystem->OnActorPoolAdded.AddUObject(Action, &UNSpawnActorBlueprintAsyncAction::OnHasPool);
		CHECK_MESSAGE(TEXT("Pre-condition: OnCreatedPoolHandle should be valid before OnHasPool runs."), Action->OnCreatedPoolHandle.IsValid())

		Action->OnHasPool(nullptr);

		CHECK_MESSAGE(TEXT("OnHasPool(null) must not reach the handle Reset() path."), Action->OnCreatedPoolHandle.IsValid())
		CHECK_MESSAGE(TEXT("Subsystem must still see the binding after OnHasPool(null) short-circuits."), Subsystem->OnActorPoolAdded.IsBound())

		Action->ConditionalBeginDestroy();
	});
}

N_TEST_HIGH(UNSpawnActorBlueprintAsyncActionTests_OnHasPool_MismatchedTemplate,
	"NEXUS::UnitTests::NActorPools::UNSpawnActorBlueprintAsyncAction::OnHasPool::MismatchedTemplate",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that OnHasPool ignores broadcasts whose pool template does not match this action's
	// ActorClass — the handle must not be reset and the subsystem binding must remain.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNActorPoolSubsystem, World)

		FNActorPoolSettings PoolSettings;
		PoolSettings.MinimumActorCount = 0;
		PoolSettings.MaximumActorCount = 1;
		PoolSettings.Strategy = ENActorPoolStrategy::Fixed;
		PoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage);

		FNActorPool OtherPool = FNActorPool(World, ANDebugActor::StaticClass(), PoolSettings);

		UNSpawnActorBlueprintAsyncAction* Action = NewObject<UNSpawnActorBlueprintAsyncAction>();
		Action->WorldContext = World;
		Action->ActorClass = TSoftClassPtr<AActor>(ANTestPooledActor::StaticClass());
		Action->OnCreatedPoolHandle = Subsystem->OnActorPoolAdded.AddUObject(Action, &UNSpawnActorBlueprintAsyncAction::OnHasPool);

		Action->OnHasPool(&OtherPool);

		CHECK_MESSAGE(TEXT("OnHasPool(mismatched) must not reach the handle Reset() path."), Action->OnCreatedPoolHandle.IsValid())
		CHECK_MESSAGE(TEXT("Subsystem must still see the binding after a mismatched-template broadcast."), Subsystem->OnActorPoolAdded.IsBound())

		OtherPool.Clear();
		Action->ConditionalBeginDestroy();
	});
}

N_TEST_HIGH(UNSpawnActorBlueprintAsyncActionTests_HandleCleanup_OnDestroy,
	"NEXUS::UnitTests::NActorPools::UNSpawnActorBlueprintAsyncAction::HandleCleanup::OnDestroy",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that BeginDestroy unregisters the OnActorPoolAdded subscription left behind when
	// no matching pool ever arrived (the H-5 handle leak) and releases the in-flight streamable
	// handle rather than leaving it to be freed only at GC (AP-R-18).
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNActorPoolSubsystem, World)

		// The developer-overlay framework (UNDeveloperOverlay) subscribes one lambda per world via
		// FWorldDelegates::OnPostWorldInitialization, so a freshly created world's subsystem can already
		// carry a subscriber whenever an overlay instance is alive (e.g. an interactive editor session).
		// Clear the delegate first so this test exercises only its own action's add/remove lifecycle.
		Subsystem->OnActorPoolAdded.Clear();
		CHECK_FALSE_MESSAGE(TEXT("Pre-condition: subsystem should start with no OnActorPoolAdded subscribers."), Subsystem->OnActorPoolAdded.IsBound())

		UNSpawnActorBlueprintAsyncAction* Action = NewObject<UNSpawnActorBlueprintAsyncAction>();
		Action->WorldContext = World;
		Action->ActorClass = TSoftClassPtr<AActor>(ANTestPooledActor::StaticClass());

		// Mirror the registration that OnLoaded() would perform once the streamable load resolves.
		Action->OnCreatedPoolHandle = Subsystem->OnActorPoolAdded.AddUObject(Action, &UNSpawnActorBlueprintAsyncAction::OnHasPool);
		CHECK_MESSAGE(TEXT("Subsystem should report a binding after the action registers."), Subsystem->OnActorPoolAdded.IsBound())

		// Plant a streamable handle to mirror the load Activate() would hold so we can observe that
		// BeginDestroy cancels and releases it.
		FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
		Action->StreamingHandle = StreamableManager.RequestAsyncLoad(Action->ActorClass.ToSoftObjectPath());
		CHECK_MESSAGE(TEXT("Pre-condition: the action should hold a streamable handle before destroy."), Action->StreamingHandle.IsValid())

		// Broadcast a non-matching pool — OnHasPool early-returns without clearing the handle,
		// which is the leak scenario BeginDestroy must clean up.
		Subsystem->CreateActorPool(ANDebugActor::StaticClass(), FNActorPoolSettings());
		CHECK_MESSAGE(TEXT("Binding should still be live after a non-matching pool was broadcast."), Subsystem->OnActorPoolAdded.IsBound())

		Action->ConditionalBeginDestroy();

		CHECK_FALSE_MESSAGE(TEXT("Subsystem must not retain a binding after the action is destroyed."), Subsystem->OnActorPoolAdded.IsBound())
		CHECK_FALSE_MESSAGE(TEXT("BeginDestroy must release the streamable handle."), Action->StreamingHandle.IsValid())
	});
}

N_TEST_HIGH(UNSpawnActorBlueprintAsyncActionTests_OnLoaded_ExpiredContext,
	"NEXUS::UnitTests::NActorPools::UNSpawnActorBlueprintAsyncAction::OnLoaded::ExpiredContext",
	N_TEST_CONTEXT_EDITOR)
{
	// Regression (AP-3): if the world context expires while the class is streaming in (a level transition
	// completing mid-load), OnLoaded must resolve the subsystem defensively and complete with null rather
	// than dereferencing a null world via Get(nullptr). Reaching the assertions below at all proves the
	// crash is gone; we also confirm it leaves no pool-created handle or subsystem subscription behind.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNActorPoolSubsystem, World)

		// Start from a known-empty delegate so we only observe this action's behavior (see HandleCleanup test
		// for why an interactive editor session can otherwise carry overlay subscribers).
		Subsystem->OnActorPoolAdded.Clear();

		UNSpawnActorBlueprintAsyncAction* Action = NewObject<UNSpawnActorBlueprintAsyncAction>();
		// Leave WorldContext unset so WorldContext.Get() resolves to null — the expired-context scenario.
		Action->ActorClass = TSoftClassPtr<AActor>(ANTestPooledActor::StaticClass());

		// The expired-context branch logs a Warning by design; register it as expected so it is treated as the
		// asserted outcome rather than stray log noise (and the test fails if the branch ever stops logging it).
		AddExpectedMessage(TEXT("world context expired"), ELogVerbosity::Warning);

		Action->OnLoaded();

		CHECK_FALSE_MESSAGE(TEXT("An expired world context must not register an OnActorPoolAdded subscription."), Subsystem->OnActorPoolAdded.IsBound())
		CHECK_FALSE_MESSAGE(TEXT("An expired world context must not leave a pending pool-created handle."), Action->OnCreatedPoolHandle.IsValid())

		Action->ConditionalBeginDestroy();
	});
}

#endif //WITH_TESTS

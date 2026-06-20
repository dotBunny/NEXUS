// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NActorPool.h"
#include "NActorPoolSubsystem.h"
#include "NGetActorBlueprintAsyncAction.h"
#include "NTestPooledActor.h"
#include "Developer/NDebugActor.h"
#include "Developer/NTestUtils.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(UNGetActorBlueprintAsyncActionTests_OnHasPool_NullPool,
	"NEXUS::UnitTests::NActorPools::UNGetActorBlueprintAsyncAction::OnHasPool::NullPool",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that the defensive guard at the top of OnHasPool short-circuits on a null pool —
	// the OnCreatedPoolHandle reset and Completed broadcast that follow must not run.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNActorPoolSubsystem, World)

		UNGetActorBlueprintAsyncAction* Action = NewObject<UNGetActorBlueprintAsyncAction>();
		Action->WorldContext = World;
		Action->ActorClass = TSoftClassPtr<AActor>(ANTestPooledActor::StaticClass());

		// Plant a real handle so we can observe whether OnHasPool would Reset() it.
		Action->OnCreatedPoolHandle = Subsystem->OnActorPoolAdded.AddUObject(Action, &UNGetActorBlueprintAsyncAction::OnHasPool);
		CHECK_MESSAGE(TEXT("Pre-condition: OnCreatedPoolHandle should be valid before OnHasPool runs."), Action->OnCreatedPoolHandle.IsValid())

		Action->OnHasPool(nullptr);

		CHECK_MESSAGE(TEXT("OnHasPool(null) must not reach the handle Reset() path."), Action->OnCreatedPoolHandle.IsValid())
		CHECK_MESSAGE(TEXT("Subsystem must still see the binding after OnHasPool(null) short-circuits."), Subsystem->OnActorPoolAdded.IsBound())

		Action->ConditionalBeginDestroy();
	});
}

N_TEST_HIGH(UNGetActorBlueprintAsyncActionTests_OnHasPool_MismatchedTemplate,
	"NEXUS::UnitTests::NActorPools::UNGetActorBlueprintAsyncAction::OnHasPool::MismatchedTemplate",
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

		UNGetActorBlueprintAsyncAction* Action = NewObject<UNGetActorBlueprintAsyncAction>();
		Action->WorldContext = World;
		Action->ActorClass = TSoftClassPtr<AActor>(ANTestPooledActor::StaticClass());
		Action->OnCreatedPoolHandle = Subsystem->OnActorPoolAdded.AddUObject(Action, &UNGetActorBlueprintAsyncAction::OnHasPool);

		Action->OnHasPool(&OtherPool);

		CHECK_MESSAGE(TEXT("OnHasPool(mismatched) must not reach the handle Reset() path."), Action->OnCreatedPoolHandle.IsValid())
		CHECK_MESSAGE(TEXT("Subsystem must still see the binding after a mismatched-template broadcast."), Subsystem->OnActorPoolAdded.IsBound())

		OtherPool.Clear();
		Action->ConditionalBeginDestroy();
	});
}

N_TEST_HIGH(UNGetActorBlueprintAsyncActionTests_HandleCleanup_OnDestroy,
	"NEXUS::UnitTests::NActorPools::UNGetActorBlueprintAsyncAction::HandleCleanup::OnDestroy",
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

		UNGetActorBlueprintAsyncAction* Action = NewObject<UNGetActorBlueprintAsyncAction>();
		Action->WorldContext = World;
		Action->ActorClass = TSoftClassPtr<AActor>(ANTestPooledActor::StaticClass());

		// Mirror the registration that OnLoaded() would perform once the streamable load resolves.
		Action->OnCreatedPoolHandle = Subsystem->OnActorPoolAdded.AddUObject(Action, &UNGetActorBlueprintAsyncAction::OnHasPool);
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

N_TEST_HIGH(UNGetActorBlueprintAsyncActionTests_OnLoaded_FailedLoad,
	"NEXUS::UnitTests::NActorPools::UNGetActorBlueprintAsyncAction::OnLoaded::FailedLoad",
	N_TEST_CONTEXT_EDITOR)
{
	// Regression: when the soft class resolves to null (deleted/renamed asset), OnLoaded must complete and
	// tear the action down instead of registering an OnActorPoolAdded callback that never fires — otherwise
	// CreateActorPool(null) returns without broadcasting and the latent node hangs / the binding leaks.
	// Observe that no subscription is left behind after a failed load.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNActorPoolSubsystem, World)

		// Start from a known-empty delegate so we only observe this action's behavior (see HandleCleanup test
		// for why an interactive editor session can otherwise carry overlay subscribers).
		Subsystem->OnActorPoolAdded.Clear();

		UNGetActorBlueprintAsyncAction* Action = NewObject<UNGetActorBlueprintAsyncAction>();
		Action->WorldContext = World;
		// A soft path to an asset that does not exist resolves to null via Get() without triggering a load.
		Action->ActorClass = TSoftClassPtr<AActor>(FSoftObjectPath(TEXT("/Game/NEXUS/DoesNotExist.DoesNotExist_C")));

		// The fix logs the failed load at Error severity (appropriate for production); the automation
		// framework auto-fails a test on an unexpected Error, so register it as expected.
		this->AddExpectedError(TEXT("soft class failed to load"), EAutomationExpectedErrorFlags::Contains, 1);

		Action->OnLoaded();

		CHECK_FALSE_MESSAGE(TEXT("A failed load must not leave an OnActorPoolAdded subscription behind."), Subsystem->OnActorPoolAdded.IsBound())
		CHECK_FALSE_MESSAGE(TEXT("A failed load must not retain a pending pool-created handle."), Action->OnCreatedPoolHandle.IsValid())

		Action->ConditionalBeginDestroy();
	});
}

#endif //WITH_TESTS

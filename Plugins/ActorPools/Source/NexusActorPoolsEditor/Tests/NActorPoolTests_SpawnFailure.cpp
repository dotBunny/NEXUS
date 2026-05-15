// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NActorPool.h"
#include "NPooledActor.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(FNActorPoolTests_CreateActor_SpawnReturnsNull,
	"NEXUS::UnitTests::NActorPools::FNActorPool::CreateActor::SpawnReturnsNull",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that FNActorPool::CreateActor handles a SpawnActorAbsolute failure gracefully —
	// here forced by templating the pool on the abstract ANPooledActor — by logging an error
	// and leaving InActors empty rather than pushing a null pointer.
	AddExpectedMessage(TEXT("SpawnActor failed because class"), ELogVerbosity::Warning);
	AddExpectedMessage(TEXT("FNActorPool failed to spawn actor of class"), ELogVerbosity::Error);

	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings Settings;
		Settings.MinimumActorCount = 1;
		Settings.MaximumActorCount = 1;
		Settings.Strategy = ENActorPoolStrategy::Fixed;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage);

		// ANPooledActor is UCLASS(Abstract); World->SpawnActorAbsolute returns nullptr for it.
		FNActorPool Pool = FNActorPool(World, ANPooledActor::StaticClass(), Settings);
		// PostInitialize -> Fill() -> CreateActors(1) -> CreateActor; spawn fails, error logged.

		CHECK_EQUALS("InActors must remain empty when CreateActor's SpawnActorAbsolute returns null.", Pool.GetAvailableCount(), 0)
		CHECK_EQUALS("OutActors must remain empty when CreateActor's SpawnActorAbsolute returns null.", Pool.GetSpawnedCount(), 0)

		Pool.Clear();
	});
}

N_TEST_HIGH(FNActorPoolTests_Spawn_PropagatesCreateFailure,
	"NEXUS::UnitTests::NActorPools::FNActorPool::Spawn::PropagatesCreateFailure",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that Spawn() on a Create-strategy pool whose template cannot be spawned by the
	// engine returns nullptr (CreateActor failed, ApplyStrategy returns false, Spawn returns null).
	AddExpectedMessage(TEXT("SpawnActor failed because class"), ELogVerbosity::Warning);
	AddExpectedMessage(TEXT("FNActorPool failed to spawn actor of class"), ELogVerbosity::Error);

	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings Settings;
		Settings.MinimumActorCount = 0;
		Settings.MaximumActorCount = 5;
		Settings.Strategy = ENActorPoolStrategy::Create;
		Settings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage);

		FNActorPool Pool = FNActorPool(World, ANPooledActor::StaticClass(), Settings);

		AActor* Result = Pool.Spawn(FVector::ZeroVector, FRotator::ZeroRotator);
		if (Result != nullptr)
		{
			ADD_ERROR("Spawn() must return nullptr when the underlying CreateActor fails.");
		}
		CHECK_EQUALS("InActors must remain empty when Spawn's CreateActor fails.", Pool.GetAvailableCount(), 0)
		CHECK_EQUALS("OutActors must remain empty when Spawn's CreateActor fails.", Pool.GetSpawnedCount(), 0)

		Pool.Clear();
	});
}

#endif //WITH_TESTS

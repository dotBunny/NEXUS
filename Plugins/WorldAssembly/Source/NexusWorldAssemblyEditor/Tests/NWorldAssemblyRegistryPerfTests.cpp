// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblyRegistry.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Organ/NOrganComponent.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NWorldAssembly::FNWorldAssemblyRegistryHarness
{
	// Number of organ components registered before each timed region.
	constexpr int32 ComponentCount = 1000;
	// Times the accessor / query is re-invoked inside the timed region (ComponentCount * Iterations element touches).
	constexpr int32 AccessorIterations = 1000;
	constexpr int32 QueryIterations = 1000;

	// Deliberately generous: this benchmark exists to capture an A/B duration (raw pointers vs TWeakObjectPtr),
	// not to gate CI. Tighten once a stable baseline is chosen.
	constexpr float MaxDuration = 100000.0f;

	/** Creates ComponentCount organ components under Owner and registers each directly with the registry. */
	static TArray<UNOrganComponent*> PopulateOrgans(AActor* Owner)
	{
		TArray<UNOrganComponent*> Components;
		Components.Reserve(ComponentCount);
		for (int32 i = 0; i < ComponentCount; ++i)
		{
			UNOrganComponent* Organ = NewObject<UNOrganComponent>(Owner);
			FNWorldAssemblyRegistry::RegisterOrganComponent(Organ);
			Components.Add(Organ);
		}
		return Components;
	}

	/** Unregisters everything PopulateOrgans added so the process-wide registry is left clean between tests. */
	static void UnpopulateOrgans(const TArray<UNOrganComponent*>& Components)
	{
		for (UNOrganComponent* Organ : Components)
		{
			FNWorldAssemblyRegistry::UnregisterOrganComponent(Organ);
		}
	}
}

class FNWorldAssemblyRegistryPerfTests
{
public:
	// Measures the cost of the public accessor as a hot caller actually pays it: snapshot + iterate, repeated.
	// Before the migration this returned a reference (zero-copy); after, it resolves a fresh TArray each call,
	// so this is the test that surfaces the accessor regression most directly.
	static void GetOrganComponents(UWorld* World)
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNWorldAssemblyRegistryHarness;
		AActor* Owner = World->SpawnActor<AActor>();
		if (Owner == nullptr) return;
		TArray<UNOrganComponent*> Components = PopulateOrgans(Owner);

		int64 Sink = 0;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNWorldAssemblyRegistryPerfTests_GetOrganComponents, MaxDuration)
			for (int32 i = 0; i < AccessorIterations; ++i)
			{
				for (const UNOrganComponent* Organ : FNWorldAssemblyRegistry::GetOrganComponents())
				{
					if (Organ != nullptr) ++Sink;
				}
			}
			NTestTimer.ManualStop();
		}

		// Keep Sink observable so the inner accumulation cannot be elided.
		UE_LOG(LogNexusWorldAssembly, Display, TEXT("GetOrganComponents touched %lld entries."), Sink);
		UnpopulateOrgans(Components);
	}

	// Measures a representative level-scoped query. Both before and after build a fresh filtered array, so this
	// isolates the marginal per-element weak-resolve cost rather than the snapshot allocation.
	static void GetOrganComponentsFromLevel(UWorld* World)
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNWorldAssemblyRegistryHarness;
		AActor* Owner = World->SpawnActor<AActor>();
		if (Owner == nullptr) return;
		TArray<UNOrganComponent*> Components = PopulateOrgans(Owner);
		const ULevel* Level = Owner->GetLevel();

		int64 Sink = 0;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNWorldAssemblyRegistryPerfTests_GetOrganComponentsFromLevel, MaxDuration)
			for (int32 i = 0; i < QueryIterations; ++i)
			{
				Sink += FNWorldAssemblyRegistry::GetOrganComponentsFromLevel(Level, false).Num();
			}
			NTestTimer.ManualStop();
		}

		UE_LOG(LogNexusWorldAssembly, Display, TEXT("GetOrganComponentsFromLevel touched %lld entries."), Sink);
		UnpopulateOrgans(Components);
	}
};

N_TEST_PERF(FNWorldAssemblyRegistryPerfTests_GetOrganComponents,
	"NEXUS::PerfTests::NWorldAssembly::FNWorldAssemblyRegistry::GetOrganComponents",
	N_TEST_CONTEXT_EDITOR)
{
	N_TESTS_PERF_START_LATENT_TEST_WORLD
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_WorldTest(&FNWorldAssemblyRegistryPerfTests::GetOrganComponents, true));
	N_TESTS_PERF_FINISH_LATENT_TEST_WORLD
}

N_TEST_PERF(FNWorldAssemblyRegistryPerfTests_GetOrganComponentsFromLevel,
	"NEXUS::PerfTests::NWorldAssembly::FNWorldAssemblyRegistry::GetOrganComponentsFromLevel",
	N_TEST_CONTEXT_EDITOR)
{
	N_TESTS_PERF_START_LATENT_TEST_WORLD
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_WorldTest(&FNWorldAssemblyRegistryPerfTests::GetOrganComponentsFromLevel, true));
	N_TESTS_PERF_FINISH_LATENT_TEST_WORLD
}

#endif //WITH_TESTS

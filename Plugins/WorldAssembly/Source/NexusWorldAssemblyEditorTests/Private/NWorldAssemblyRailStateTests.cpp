// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "EdMode/NWorldAssemblyEdModeRail.h"
#include "EdMode/NWorldAssemblyRailState.h"
#include "Framework/Commands/UICommandList.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyRailStateHarness
{
	/** Positions on the production rail, in the order RegisterRails adds the categories: World, Cell, Cell Data,
	 *  Junction, Organ. */
	constexpr int32 WorldIndex = 0;
	constexpr int32 CellIndex = 1;
	constexpr int32 OrganIndex = 4;

	/** What one stand-in category answers with. */
	struct FTestRailDefinition
	{
		/** Whether the level holds anything for this category to act on. */
		bool bAvailable = false;

		/** Whether the category asks to be opened on when it is available. */
		bool bAutoSelect = false;
	};

	/**
	 * Stand-in for a category, answering the two questions the rail state asks with fixed values.
	 *
	 * The production rails answer both by walking the level, which is exactly what this replaces: what is under test is
	 * the order the state resolves those answers in, not the predicates themselves.
	 */
	class FTestRail final : public FNWorldAssemblyEdModeRail
	{
	public:
		FTestRail(const TSharedRef<FUICommandList>& InCommandList, const FTestRailDefinition& InDefinition)
			: FNWorldAssemblyEdModeRail(InCommandList), Definition(InDefinition) {}

		//~FNWorldAssemblyEdModeRail
		/** @return Null; nothing in the rail state reads the category command, only the strip that draws it does. */
		virtual TSharedPtr<FUICommandInfo> GetCategoryCommand() const override { return nullptr; }

		virtual TAttribute<bool> GetAvailable() const override { return TAttribute<bool>(Definition.bAvailable); }
		virtual bool ShouldAutoSelect() const override { return Definition.bAutoSelect; }
		//End FNWorldAssemblyEdModeRail

	private:
		const FTestRailDefinition Definition;
	};

	/**
	 * @param Definitions The categories to build, in rail order.
	 * @return The category a freshly opened rail state lands on, or INDEX_NONE when none is available.
	 */
	int32 GetSeededIndex(const TArray<FTestRailDefinition>& Definitions)
	{
		// One list shared by every stand-in, as the toolkit's is: a rail resolves its buttons against the union rather
		// than against a list of its own.
		const TSharedRef<FUICommandList> CommandList = MakeShared<FUICommandList>();

		TArray<TSharedRef<FNWorldAssemblyEdModeRail>> Rails;
		Rails.Reserve(Definitions.Num());
		for (const FTestRailDefinition& Definition : Definitions)
		{
			Rails.Add(MakeShared<FTestRail>(CommandList, Definition));
		}

		// Seeding is the constructor's whole job, so the state is read where it is built.
		return FNWorldAssemblyRailState(MoveTemp(Rails)).GetActiveIndex();
	}

	/**
	 * @param bHasCellActor Whether the level holds an ANCellActor.
	 * @param bHasOrgan Whether the level holds an organ component.
	 * @return The production rail layout as it stands for that level: World always available and never asking to be
	 *         opened on, Cell/Cell Data/Junction keyed on the cell actor with only Cell asking, Organ on the organ.
	 */
	TArray<FTestRailDefinition> MakeProductionRails(const bool bHasCellActor, const bool bHasOrgan)
	{
		return {
			{ true,          false },
			{ bHasCellActor, true  },
			{ bHasCellActor, false },
			{ bHasCellActor, false },
			{ bHasOrgan,     true  },
		};
	}
}

N_TEST_HIGH(FNWorldAssemblyRailStateTests_Seed_CellLevelOpensOnCell,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyRailState::Seed::CellLevelOpensOnCell",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyRailStateHarness;

	// Verifies a level holding a cell actor opens on Cell rather than on World, which is available first.
	CHECK_EQUALS("A level with a cell actor should open on the Cell category.",
		GetSeededIndex(MakeProductionRails(true, false)), CellIndex)
}

N_TEST_HIGH(FNWorldAssemblyRailStateTests_Seed_OrganLevelOpensOnOrgan,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyRailState::Seed::OrganLevelOpensOnOrgan",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyRailStateHarness;

	// Verifies a level holding organs opens on Organ, which sits last on the rail with three hidden categories above it.
	CHECK_EQUALS("A level with an organ should open on the Organ category.",
		GetSeededIndex(MakeProductionRails(false, true)), OrganIndex)
}

N_TEST_MEDIUM(FNWorldAssemblyRailStateTests_Seed_EmptyLevelOpensOnWorld,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyRailState::Seed::EmptyLevelOpensOnWorld",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyRailStateHarness;

	// Verifies a level with neither still falls to the first available category, which is where World is offered.
	CHECK_EQUALS("A level with no cell and no organ should open on the World category.",
		GetSeededIndex(MakeProductionRails(false, false)), WorldIndex)
}

N_TEST_MEDIUM(FNWorldAssemblyRailStateTests_Seed_CellWinsOverOrgan,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyRailState::Seed::CellWinsOverOrgan",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyRailStateHarness;

	// Verifies rail order settles a level that is somehow both. The add commands refuse to author one, so this only
	// arises in data predating that rule — but it has to resolve to something, and Cell is where the mode can act.
	CHECK_EQUALS("A level holding both should open on Cell, which sits above Organ on the rail.",
		GetSeededIndex(MakeProductionRails(true, true)), CellIndex)
}

N_TEST_MEDIUM(FNWorldAssemblyRailStateTests_Seed_UnavailableCategoryIsNotOpenedOn,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyRailState::Seed::UnavailableCategoryIsNotOpenedOn",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyRailStateHarness;

	// Verifies asking to be opened on does not override being hidden — a category whose button the strip is not drawing
	// would leave the panel showing content the user has no way back to.
	const TArray<FTestRailDefinition> Rails = {
		{ true,  false },
		{ false, true  },
	};

	CHECK_EQUALS("An unavailable category should be passed over however much it wants the selection.",
		GetSeededIndex(Rails), 0)
}

N_TEST_LOW(FNWorldAssemblyRailStateTests_Seed_NothingAvailableSelectsNothing,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyRailState::Seed::NothingAvailableSelectsNothing",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyRailStateHarness;

	// Verifies the empty answer the panel clamps against. World carries no availability predicate, so the production
	// rail cannot reach this — the panel's switcher still assumes it, and this is what pins that assumption.
	const TArray<FTestRailDefinition> Rails = {
		{ false, false },
		{ false, true  },
	};

	CHECK_EQUALS("No available category should leave nothing selected.", GetSeededIndex(Rails), INDEX_NONE)
}

#endif //WITH_TESTS

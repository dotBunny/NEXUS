// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NSubsystemMacrosTestTypes.h"
#include "Developer/NTestUtils.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::UnitTests::NCore::NSubsystemMacrosHarness
{
	/** Sets FNSubsystemMacrosTestSwitch for a scope and turns it back off on the way out. */
	struct FScopedAllowCreation
	{
		explicit FScopedAllowCreation(const bool bAllow)
		{
			*FNSubsystemMacrosTestSwitch::Get() = bAllow;
		}

		~FScopedAllowCreation()
		{
			*FNSubsystemMacrosTestSwitch::Get() = false;
		}

		FScopedAllowCreation(const FScopedAllowCreation&) = delete;
		FScopedAllowCreation& operator=(const FScopedAllowCreation&) = delete;
	};

	/**
	 * Creates a GamePreview world (neither Game nor PIE, and a world type cooked builds do create), runs a body
	 * against it, and destroys it.
	 */
	static void PreviewWorldTest(const TFunctionRef<void(UWorld* World)>& TestFunctionality)
	{
		constexpr bool bInformEngineOfWorld = false;
		UWorld* World = UWorld::CreateWorld(EWorldType::GamePreview, bInformEngineOfWorld, TEXT("NTestPreviewWorld"));
		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::GamePreview);
		WorldContext.SetCurrentWorld(World);

		TestFunctionality(World);

		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(bInformEngineOfWorld);
	}
}

N_TEST_CRITICAL(NSubsystemMacrosTests_WorldSubsystemGameOnly_CreatedInGameWorld,
	"NEXUS::UnitTests::NCore::NSubsystemMacros::WorldSubsystemGameOnly::CreatedInGameWorld",
	N_TEST_CONTEXT_EDITOR)
{
	const NEXUS::UnitTests::NCore::NSubsystemMacrosHarness::FScopedAllowCreation Allow(true);
	FNTestUtils::WorldTest(EWorldType::Game, [this](UWorld* World)
	{
		CHECK_MESSAGE(TEXT("A game world should create the subsystem when ShouldCreate holds."),
			UNSubsystemMacrosTestWorldSubsystem::Get(World) != nullptr)
	});
}

N_TEST_CRITICAL(NSubsystemMacrosTests_WorldSubsystemGameOnly_NotCreatedWhenDisallowed,
	"NEXUS::UnitTests::NCore::NSubsystemMacros::WorldSubsystemGameOnly::NotCreatedWhenDisallowed",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that a conditional ShouldCreate is evaluated as a whole. Expanded without parentheses, the fixture's
	// "(Switch != nullptr) ? *Switch : false" read as "(!(Switch != nullptr)) ? *Switch : false", which is false
	// whenever the switch exists, so the macro never refused and the subsystem was created regardless.
	const NEXUS::UnitTests::NCore::NSubsystemMacrosHarness::FScopedAllowCreation Allow(false);
	FNTestUtils::WorldTest(EWorldType::Game, [this](UWorld* World)
	{
		CHECK_MESSAGE(TEXT("A game world should not create the subsystem when ShouldCreate is false."),
			UNSubsystemMacrosTestWorldSubsystem::Get(World) == nullptr)
	});
}

N_TEST_CRITICAL(NSubsystemMacrosTests_WorldSubsystemGameOnly_NotCreatedInPreviewWorld,
	"NEXUS::UnitTests::NCore::NSubsystemMacros::WorldSubsystemGameOnly::NotCreatedInPreviewWorld",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that only Game and PIE worlds create the subsystem. The non-editor build once created it whenever the
	// base class declined, which is how a cooked game came to create it in world types like this one.
	const NEXUS::UnitTests::NCore::NSubsystemMacrosHarness::FScopedAllowCreation Allow(true);
	NEXUS::UnitTests::NCore::NSubsystemMacrosHarness::PreviewWorldTest([this](UWorld* World)
	{
		CHECK_MESSAGE(TEXT("A GamePreview world should not create the subsystem."),
			UNSubsystemMacrosTestWorldSubsystem::Get(World) == nullptr)
	});
}

N_TEST_CRITICAL(NSubsystemMacrosTests_TickableWorldSubsystemGameOnly_CreatedInGameWorld,
	"NEXUS::UnitTests::NCore::NSubsystemMacros::TickableWorldSubsystemGameOnly::CreatedInGameWorld",
	N_TEST_CONTEXT_EDITOR)
{
	const NEXUS::UnitTests::NCore::NSubsystemMacrosHarness::FScopedAllowCreation Allow(true);
	FNTestUtils::WorldTest(EWorldType::Game, [this](UWorld* World)
	{
		CHECK_MESSAGE(TEXT("A game world should create the tickable subsystem when ShouldCreate holds."),
			UNSubsystemMacrosTestTickableWorldSubsystem::Get(World) != nullptr)
	});
}

N_TEST_CRITICAL(NSubsystemMacrosTests_TickableWorldSubsystemGameOnly_NotCreatedWhenDisallowed,
	"NEXUS::UnitTests::NCore::NSubsystemMacros::TickableWorldSubsystemGameOnly::NotCreatedWhenDisallowed",
	N_TEST_CONTEXT_EDITOR)
{
	const NEXUS::UnitTests::NCore::NSubsystemMacrosHarness::FScopedAllowCreation Allow(false);
	FNTestUtils::WorldTest(EWorldType::Game, [this](UWorld* World)
	{
		CHECK_MESSAGE(TEXT("A game world should not create the tickable subsystem when ShouldCreate is false."),
			UNSubsystemMacrosTestTickableWorldSubsystem::Get(World) == nullptr)
	});
}

N_TEST_CRITICAL(NSubsystemMacrosTests_TickableWorldSubsystemGameOnly_NotCreatedInPreviewWorld,
	"NEXUS::UnitTests::NCore::NSubsystemMacros::TickableWorldSubsystemGameOnly::NotCreatedInPreviewWorld",
	N_TEST_CONTEXT_EDITOR)
{
	const NEXUS::UnitTests::NCore::NSubsystemMacrosHarness::FScopedAllowCreation Allow(true);
	NEXUS::UnitTests::NCore::NSubsystemMacrosHarness::PreviewWorldTest([this](UWorld* World)
	{
		CHECK_MESSAGE(TEXT("A GamePreview world should not create the tickable subsystem."),
			UNSubsystemMacrosTestTickableWorldSubsystem::Get(World) == nullptr)
	});
}

#endif //WITH_TESTS

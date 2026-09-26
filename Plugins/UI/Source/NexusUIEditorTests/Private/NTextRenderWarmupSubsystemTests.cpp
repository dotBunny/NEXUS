// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Editor.h"
#include "NTextRenderWarmupSubsystem.h"
#include "NUISettings.h"
#include "Components/TextRenderComponent.h"
#include "Developer/NTestUtils.h"
#include "Engine/Font.h"
#include "Macros/NTestMacros.h"
#include "Misc/App.h"
#include "UObject/UObjectIterator.h"

namespace NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystemHarness
{
	/** Creates a font nothing else uses, so the pair it makes belongs to the test alone. */
	static UFont* MakeFont(const EFontCacheType CacheType)
	{
		UFont* Font = NewObject<UFont>(GetTransientPackage(), NAME_None, RF_Transient);
		Font->FontCacheType = CacheType;
		return Font;
	}

	/** Creates an unregistered text render drawing with a font, as a level that has loaded but not registered holds one. */
	static UTextRenderComponent* MakeLoadedTextRender(UFont* Font)
	{
		UTextRenderComponent* Component = NewObject<UTextRenderComponent>(GetTransientPackage(), NAME_None, RF_Transient);
		Component->SetFont(Font);
		return Component;
	}

	/** Sets UNUISettings::bWarmTextRenderMaterials for a scope, putting the configured value back on the way out. */
	struct FScopedWarmSetting
	{
		explicit FScopedWarmSetting(const bool bValue)
			: bPrevious(UNUISettings::Get()->bWarmTextRenderMaterials)
		{
			UNUISettings::GetMutable()->bWarmTextRenderMaterials = bValue;
		}

		~FScopedWarmSetting()
		{
			UNUISettings::GetMutable()->bWarmTextRenderMaterials = bPrevious;
		}

		FScopedWarmSetting(const FScopedWarmSetting&) = delete;
		FScopedWarmSetting& operator=(const FScopedWarmSetting&) = delete;

	private:
		bool bPrevious;
	};
}

N_TEST_HIGH(UNTextRenderWarmupSubsystemTests_ShouldCreate_DisabledBySetting,
	"NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystem::ShouldCreate::DisabledBySetting",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that turning the setting off keeps the subsystem out of a game world, rendering or not.
	const NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystemHarness::FScopedWarmSetting Setting(false);
	FNTestUtils::WorldTest(EWorldType::Game, [this](UWorld* World)
	{
		CHECK_MESSAGE(TEXT("The subsystem should not be created while Warm Text Render Materials is off."),
			UNTextRenderWarmupSubsystem::Get(World) == nullptr)
	});
}

N_TEST_HIGH(UNTextRenderWarmupSubsystemTests_ShouldCreate_GameWorldWhenRendering,
	"NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystem::ShouldCreate::GameWorldWhenRendering",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that a game world has the subsystem exactly when this process can render: never under -nullrhi or on a
	// dedicated server, where no text render makes a proxy.
	const NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystemHarness::FScopedWarmSetting Setting(true);
	FNTestUtils::WorldTest(EWorldType::Game, [this](UWorld* World)
	{
		CHECK_EQUALS("A game world should have the subsystem exactly when the process can render.",
			UNTextRenderWarmupSubsystem::Get(World) != nullptr, FApp::CanEverRender())
	});
}

N_TEST_HIGH(UNTextRenderWarmupSubsystemTests_ShouldCreate_NotInEditorWorld,
	"NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystem::ShouldCreate::NotInEditorWorld",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that the editor's own world is left alone; only Game and PIE worlds are warmed.
	const UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	if (EditorWorld == nullptr)
	{
		ADD_ERROR("The editor has no editor world.");
		return;
	}
	CHECK_MESSAGE(TEXT("The editor world should not have the subsystem."), UNTextRenderWarmupSubsystem::Get(EditorWorld) == nullptr)
}

N_TEST_CRITICAL(UNTextRenderWarmupSubsystemTests_Warm_OfflineFont,
	"NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystem::Warm::OfflineFont",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that a loaded text render drawing with an offline font has its pair warmed, and only once however many
	// text renders share it. Without an RHI nothing renders and there is nothing to warm, so there the test checks the
	// subsystem stayed away instead.
	const NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystemHarness::FScopedWarmSetting Setting(true);
	FNTestUtils::WorldTest(EWorldType::Game, [this](UWorld* World)
	{
		UNTextRenderWarmupSubsystem* Subsystem = UNTextRenderWarmupSubsystem::Get(World);
		if (!FApp::CanEverRender())
		{
			CHECK_MESSAGE(TEXT("The subsystem should not exist where nothing renders."), Subsystem == nullptr)
			return;
		}
		if (Subsystem == nullptr)
		{
			ADD_ERROR("Could not retrieve UNTextRenderWarmupSubsystem from a rendering game world.");
			return;
		}

		UFont* Font = NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystemHarness::MakeFont(EFontCacheType::Offline);
		const UTextRenderComponent* First = NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystemHarness::MakeLoadedTextRender(Font);
		NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystemHarness::MakeLoadedTextRender(Font);

		CHECK_FALSE_MESSAGE(TEXT("The pair should not be warm before the subsystem has looked."),
			Subsystem->IsWarm(First->TextMaterial, Font))

		const int32 PairsBefore = Subsystem->GetWarmPairCount();
		Subsystem->WarmLoadedTextRenders();

		CHECK_MESSAGE(TEXT("A loaded text render's offline font pair should be warm."),
			Subsystem->IsWarm(First->TextMaterial, Font))
		CHECK_EQUALS("Two text renders sharing a pair should warm it once.",
			Subsystem->GetWarmPairCount(), PairsBefore + 1)

		Subsystem->WarmLoadedTextRenders();
		CHECK_EQUALS("Looking again should warm nothing new.", Subsystem->GetWarmPairCount(), PairsBefore + 1)
	}, true);
}

N_TEST_HIGH(UNTextRenderWarmupSubsystemTests_Warm_RuntimeFontSkipped,
	"NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystem::Warm::RuntimeFontSkipped",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that a runtime font, which never draws through the MID cache, is not warmed.
	const NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystemHarness::FScopedWarmSetting Setting(true);
	FNTestUtils::WorldTest(EWorldType::Game, [this](UWorld* World)
	{
		UNTextRenderWarmupSubsystem* Subsystem = UNTextRenderWarmupSubsystem::Get(World);
		if (Subsystem == nullptr)
		{
			CHECK_FALSE_MESSAGE(TEXT("A rendering game world should have the subsystem."), FApp::CanEverRender())
			return;
		}

		UFont* Font = NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystemHarness::MakeFont(EFontCacheType::Runtime);
		const UTextRenderComponent* Loaded = NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystemHarness::MakeLoadedTextRender(Font);
		Subsystem->WarmLoadedTextRenders();

		CHECK_FALSE_MESSAGE(TEXT("A runtime font's pair should not be warmed."), Subsystem->IsWarm(Loaded->TextMaterial, Font))
	}, true);
}

N_TEST_HIGH(UNTextRenderWarmupSubsystemTests_Release_OnWorldCleanup,
	"NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystem::Release::OnWorldCleanup",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that no warm text render is left registered once its world is gone; the world reports an ownerless
	// component still registered at cleanup as leaked.
	const NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystemHarness::FScopedWarmSetting Setting(true);
	const UWorld* TestWorld = nullptr;
	FNTestUtils::WorldTest(EWorldType::Game, [this, &TestWorld](UWorld* World)
	{
		TestWorld = World;
		UNTextRenderWarmupSubsystem* Subsystem = UNTextRenderWarmupSubsystem::Get(World);
		if (Subsystem == nullptr)
		{
			CHECK_FALSE_MESSAGE(TEXT("A rendering game world should have the subsystem."), FApp::CanEverRender())
			return;
		}

		UFont* Font = NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystemHarness::MakeFont(EFontCacheType::Offline);
		const UTextRenderComponent* Loaded = NEXUS::UnitTests::NUI::UNTextRenderWarmupSubsystemHarness::MakeLoadedTextRender(Font);
		Subsystem->WarmLoadedTextRenders();
		CHECK_MESSAGE(TEXT("The pair should be warm before the world goes."), Subsystem->IsWarm(Loaded->TextMaterial, Font))
	}, true);

	for (const UTextRenderComponent* Component : TObjectRange<UTextRenderComponent>())
	{
		if (Component->GetOuter() == TestWorld && Component->IsRegistered())
		{
			ADD_ERROR(FString::Printf(TEXT("%s is still registered after its world was destroyed."), *Component->GetName()));
		}
	}
}

#endif //WITH_TESTS

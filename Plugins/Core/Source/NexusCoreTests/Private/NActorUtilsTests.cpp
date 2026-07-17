// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NActorUtils.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Developer/NDebugActor.h"
#include "Developer/NTestUtils.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/PlayerStart.h"
#include "Macros/NTestMacros.h"
#include "UObject/StrongObjectPtr.h"

namespace NEXUS::UnitTests::NCore::FNActorUtilsHarness
{
	/** Tag applied to actors spawned by these tests so the ExclusionFunction can isolate them from any pre-existing world actors. */
	static const FName TestTag = FName(TEXT("FNActorUtilsTest"));

	/** Builds a filter that only keeps actors carrying the test tag. */
	static FNWorldActorFilterSettings MakeTagFilter()
	{
		FNWorldActorFilterSettings Settings;
		Settings.ExclusionFunction = [](const AActor* Actor) { return Actor->Tags.Contains(TestTag); };
		return Settings;
	}

	/**
	 * Build a USCS_Node parented to OwningSCS. When ComponentClass is non-null a transient template instance of that class is
	 * created and assigned; passing nullptr leaves ComponentTemplate null — useful for simulating SCS nodes whose underlying
	 * component class has been deleted or has not yet been regenerated.
	 */
	static USCS_Node* MakeNode(USimpleConstructionScript* OwningSCS, UClass* ComponentClass)
	{
		USCS_Node* Node = NewObject<USCS_Node>(OwningSCS, NAME_None, RF_Transient);
		if (ComponentClass != nullptr)
		{
			Node->ComponentClass = ComponentClass;
			Node->ComponentTemplate = NewObject<UActorComponent>(OwningSCS, ComponentClass, NAME_None, RF_Transient);
		}
		return Node;
	}

	/**
	 * Build a synthetic UBlueprintGeneratedClass parented to SuperClass, with an SCS populated by Populate.
	 * The class is never instantiated or asked for a CDO, so Bind/StaticLink are deliberately skipped — only the SCS
	 * traversal path of FNActorUtils::GetRootComponentFromDefaultObject is exercised through it. The synthetic class has
	 * no owning UBlueprint, so USimpleConstructionScript::ValidateSceneRootNodes (invoked from AddNode) is a no-op and
	 * does not auto-insert a DefaultSceneRoot we did not ask for.
	 */
	static UBlueprintGeneratedClass* MakeSyntheticBPGC(UClass* SuperClass, TFunctionRef<void(USimpleConstructionScript*)> Populate)
	{
		UBlueprintGeneratedClass* BPGC = NewObject<UBlueprintGeneratedClass>(
			GetTransientPackage(), UBlueprintGeneratedClass::StaticClass(), NAME_None, RF_Transient);
		BPGC->SetSuperStruct(SuperClass);
		BPGC->SimpleConstructionScript = NewObject<USimpleConstructionScript>(BPGC, NAME_None, RF_Transient);
		Populate(BPGC->SimpleConstructionScript);
		return BPGC;
	}
}

N_TEST_HIGH(FNActorUtilsTests_GetWorldActors_NullWorld_ReturnsEmpty,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetWorldActors::NullWorld_ReturnsEmpty",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Header contract: "A null world yields an empty array." TActorIterator on a null world should not
	// crash and GetWorldActors must short-circuit cleanly.
	const FNWorldActorFilterSettings Settings;
	const TArray<AActor*> Result = FNActorUtils::GetWorldActors(nullptr, Settings);
	CHECK_EQUALS("Null world must yield zero actors", Result.Num(), 0);
}

N_TEST_HIGH(FNActorUtilsTests_GetWorldActors_DefaultSettings_IncludesSpawnedActor,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetWorldActors::DefaultSettings_IncludesSpawnedActor",
	N_TEST_CONTEXT_EDITOR)
{
	// Baseline path — default settings, no filtering — spawn a tagged actor and confirm it appears in the
	// result. Uses the test-tag filter to isolate from any pre-existing world actors.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		ANDebugActor* Spawned = World->SpawnActor<ANDebugActor>();
		if (Spawned == nullptr)
		{
			ADD_ERROR("Failed to spawn ANDebugActor in the test world");
			return;
		}
		Spawned->Tags.Add(NEXUS::UnitTests::NCore::FNActorUtilsHarness::TestTag);

		const TArray<AActor*> Result = FNActorUtils::GetWorldActors(
			World, NEXUS::UnitTests::NCore::FNActorUtilsHarness::MakeTagFilter());

		CHECK_EQUALS("Default settings must include exactly the one tagged actor we spawned", Result.Num(), 1);
		if (Result.Num() == 1)
		{
			CHECK_MESSAGE(TEXT("Returned actor must be the one we spawned"), Result[0] == Spawned);
		}
	});
}

N_TEST_HIGH(FNActorUtilsTests_GetWorldActors_ExclusionFunction_FiltersByPredicate,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetWorldActors::ExclusionFunction_FiltersByPredicate",
	N_TEST_CONTEXT_EDITOR)
{
	// Spawn two tagged actors; the predicate keeps only the one with a "Special" tag. Validates the
	// "return true to keep, false to exclude" contract on the ExclusionFunction.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		ANDebugActor* Plain = World->SpawnActor<ANDebugActor>();
		ANDebugActor* Special = World->SpawnActor<ANDebugActor>();
		if (Plain == nullptr || Special == nullptr)
		{
			ADD_ERROR("Failed to spawn one of the test actors");
			return;
		}
		Plain->Tags.Add(NEXUS::UnitTests::NCore::FNActorUtilsHarness::TestTag);
		Special->Tags.Add(NEXUS::UnitTests::NCore::FNActorUtilsHarness::TestTag);
		Special->Tags.Add(FName(TEXT("Special")));

		FNWorldActorFilterSettings Settings;
		Settings.ExclusionFunction = [](const AActor* Actor)
		{
			return Actor->Tags.Contains(NEXUS::UnitTests::NCore::FNActorUtilsHarness::TestTag)
				&& Actor->Tags.Contains(FName(TEXT("Special")));
		};

		const TArray<AActor*> Result = FNActorUtils::GetWorldActors(World, Settings);
		CHECK_EQUALS("Predicate must keep only the Special-tagged actor", Result.Num(), 1);
		if (Result.Num() == 1)
		{
			CHECK_MESSAGE(TEXT("Returned actor must be the Special-tagged one"), Result[0] == Special);
		}
	});
}

N_TEST_HIGH(FNActorUtilsTests_GetWorldActors_ExcludeNonCollisionEnabled_SkipsDisabledActor,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetWorldActors::ExcludeNonCollisionEnabled_SkipsDisabledActor",
	N_TEST_CONTEXT_EDITOR)
{
	// bExcludeNonCollisionEnabledActors must skip actors whose collision is disabled at the actor level.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		ANDebugActor* EnabledActor = World->SpawnActor<ANDebugActor>();
		ANDebugActor* DisabledActor = World->SpawnActor<ANDebugActor>();
		if (EnabledActor == nullptr || DisabledActor == nullptr)
		{
			ADD_ERROR("Failed to spawn one of the test actors");
			return;
		}
		EnabledActor->Tags.Add(NEXUS::UnitTests::NCore::FNActorUtilsHarness::TestTag);
		DisabledActor->Tags.Add(NEXUS::UnitTests::NCore::FNActorUtilsHarness::TestTag);
		DisabledActor->SetActorEnableCollision(false);

		FNWorldActorFilterSettings Settings = NEXUS::UnitTests::NCore::FNActorUtilsHarness::MakeTagFilter();
		Settings.bExcludeNonCollisionEnabledActors = true;

		const TArray<AActor*> Result = FNActorUtils::GetWorldActors(World, Settings);
		CHECK_EQUALS("Collision-disabled actor must be excluded; only the enabled one survives", Result.Num(), 1);
		if (Result.Num() == 1)
		{
			CHECK_MESSAGE(TEXT("Returned actor must be the one with collision enabled"), Result[0] == EnabledActor);
		}
	});
}

N_TEST_HIGH(FNActorUtilsTests_GetWorldActors_IncludePlayerStarts_OverridesExclusion,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetWorldActors::IncludePlayerStarts_OverridesExclusion",
	N_TEST_CONTEXT_EDITOR)
{
	// bIncludePlayerStarts short-circuits every other filter. With an ExclusionFunction that rejects
	// everything, the player start must still come through; the regular actor must not.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		APlayerStart* Start = World->SpawnActor<APlayerStart>();
		ANDebugActor* Plain = World->SpawnActor<ANDebugActor>();
		if (Start == nullptr || Plain == nullptr)
		{
			ADD_ERROR("Failed to spawn one of the test actors");
			return;
		}
		Start->Tags.Add(NEXUS::UnitTests::NCore::FNActorUtilsHarness::TestTag);
		Plain->Tags.Add(NEXUS::UnitTests::NCore::FNActorUtilsHarness::TestTag);

		FNWorldActorFilterSettings Settings;
		Settings.bIncludePlayerStarts = true;
		// Predicate that rejects every actor — only the PlayerStart short-circuit should add anything.
		Settings.ExclusionFunction = [](const AActor*) { return false; };

		const TArray<AActor*> Result = FNActorUtils::GetWorldActors(World, Settings);

		bool bFoundStart = false;
		bool bFoundPlain = false;
		for (AActor* A : Result)
		{
			if (A == Start) bFoundStart = true;
			if (A == Plain) bFoundPlain = true;
		}
		CHECK_MESSAGE(TEXT("PlayerStart must bypass the rejecting ExclusionFunction"), bFoundStart);
		CHECK_FALSE_MESSAGE(TEXT("Plain actor must still be excluded by the rejecting ExclusionFunction"), bFoundPlain);
	});
}

N_TEST_HIGH(FNActorUtilsTests_GetWorldActors_IncludePlayerStartsFalse_HonoursExclusion,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetWorldActors::IncludePlayerStartsFalse_HonoursExclusion",
	N_TEST_CONTEXT_EDITOR)
{
	// Mirror of the previous test — with bIncludePlayerStarts=false the PlayerStart must be subject to
	// the regular filters and a rejecting ExclusionFunction must exclude it.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		APlayerStart* Start = World->SpawnActor<APlayerStart>();
		if (Start == nullptr)
		{
			ADD_ERROR("Failed to spawn APlayerStart");
			return;
		}
		Start->Tags.Add(NEXUS::UnitTests::NCore::FNActorUtilsHarness::TestTag);

		FNWorldActorFilterSettings Settings;
		Settings.bIncludePlayerStarts = false;
		Settings.ExclusionFunction = [](const AActor*) { return false; };

		const TArray<AActor*> Result = FNActorUtils::GetWorldActors(World, Settings);

		bool bFoundStart = false;
		for (AActor* A : Result)
		{
			if (A == Start) bFoundStart = true;
		}
		CHECK_FALSE_MESSAGE(TEXT("PlayerStart must be excluded when bIncludePlayerStarts==false and the predicate rejects"), bFoundStart);
	});
}

N_TEST_HIGH(FNActorUtilsTests_GetRootComponentFromDefaultObject_NativeActor_ReturnsCDORoot,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetRootComponentFromDefaultObject::NativeActor_ReturnsCDORoot",
	N_TEST_CONTEXT_ANYWHERE)
{
	// For a native C++ AActor subclass (no UBlueprintGeneratedClass) the function short-circuits at the
	// first null check and returns the CDO's root component. ANDebugActor's constructor sets up a sphere
	// mesh as its root, so the CDO must have a non-null RootComponent.
	USceneComponent* Root = FNActorUtils::GetRootComponentFromDefaultObject(ANDebugActor::StaticClass());
	if (Root == nullptr)
	{
		ADD_ERROR("ANDebugActor's CDO unexpectedly has no root component");
		return;
	}
	CHECK_MESSAGE(TEXT("Native-class lookup must return the CDO's root component"), Root != nullptr);
}

N_TEST_HIGH(FNActorUtilsTests_GetRootComponentFromDefaultObject_PlainAActor_Null,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetRootComponentFromDefaultObject::PlainAActor_Null",
	N_TEST_CONTEXT_ANYWHERE)
{
	// AActor's own CDO has no root component by default, so the same short-circuit returns nullptr —
	// callers must be prepared to handle this case (the docstring on FNActorUtils.h:50 warns about it
	// indirectly via the "nullptr if none is found" return contract).
	USceneComponent* Root = FNActorUtils::GetRootComponentFromDefaultObject(AActor::StaticClass());
	CHECK_MESSAGE(TEXT("AActor::StaticClass() CDO has no root component → expect nullptr"), Root == nullptr);
}

N_TEST_HIGH(FNActorUtilsTests_GetRootComponentFromDefaultObject_NullClass_ReturnsNull,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetRootComponentFromDefaultObject::NullClass_ReturnsNull",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A null TSubclassOf<AActor> must short-circuit to nullptr — defensive callers (NActorPool::PreInitialize, etc.)
	// rely on this contract instead of pre-validating the class themselves.
	const TSubclassOf<AActor> NullClass = nullptr;
	USceneComponent* Root = FNActorUtils::GetRootComponentFromDefaultObject(NullClass);
	CHECK_MESSAGE(TEXT("Null TSubclassOf must return nullptr without crashing"), Root == nullptr);
}

N_TEST_HIGH(FNActorUtilsTests_GetRootComponentFromDefaultObject_NullSCSTemplate_Skipped,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetRootComponentFromDefaultObject::NullSCSTemplate_Skipped",
	N_TEST_CONTEXT_ANYWHERE)
{
	// First root node carries a null ComponentTemplate (mirrors an SCS node whose component class was deleted or has
	// not been regenerated). The second node has a real USceneComponent template — the function must skip the null
	// one without crashing and return the scene template.
	USceneComponent* ExpectedTemplate = nullptr;
	UBlueprintGeneratedClass* BPGC = NEXUS::UnitTests::NCore::FNActorUtilsHarness::MakeSyntheticBPGC(
		AActor::StaticClass(),
		[&ExpectedTemplate](USimpleConstructionScript* SCS)
		{
			SCS->AddNode(NEXUS::UnitTests::NCore::FNActorUtilsHarness::MakeNode(SCS, nullptr));
			USCS_Node* SceneNode = NEXUS::UnitTests::NCore::FNActorUtilsHarness::MakeNode(SCS, USceneComponent::StaticClass());
			SCS->AddNode(SceneNode);
			ExpectedTemplate = Cast<USceneComponent>(SceneNode->ComponentTemplate);
		});
	const TStrongObjectPtr<UBlueprintGeneratedClass> GCGuard(BPGC);

	USceneComponent* Root = FNActorUtils::GetRootComponentFromDefaultObject(TSubclassOf<AActor>(BPGC));
	CHECK_MESSAGE(TEXT("Null ComponentTemplate must be skipped and the next scene template returned"),
		Root != nullptr && Root == ExpectedTemplate);
}

N_TEST_HIGH(FNActorUtilsTests_GetRootComponentFromDefaultObject_NonSceneRoot_Skipped,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetRootComponentFromDefaultObject::NonSceneRoot_Skipped",
	N_TEST_CONTEXT_ANYWHERE)
{
	// First root node holds a non-scene UActorComponent template (UInputComponent); the second holds a USceneComponent.
	// The function must iterate past the non-scene one rather than aborting at RootNodes[0].
	USceneComponent* ExpectedTemplate = nullptr;
	UBlueprintGeneratedClass* BPGC = NEXUS::UnitTests::NCore::FNActorUtilsHarness::MakeSyntheticBPGC(
		AActor::StaticClass(),
		[&ExpectedTemplate](USimpleConstructionScript* SCS)
		{
			SCS->AddNode(NEXUS::UnitTests::NCore::FNActorUtilsHarness::MakeNode(SCS, UInputComponent::StaticClass()));
			USCS_Node* SceneNode = NEXUS::UnitTests::NCore::FNActorUtilsHarness::MakeNode(SCS, USceneComponent::StaticClass());
			SCS->AddNode(SceneNode);
			ExpectedTemplate = Cast<USceneComponent>(SceneNode->ComponentTemplate);
		});
	const TStrongObjectPtr<UBlueprintGeneratedClass> GCGuard(BPGC);

	USceneComponent* Root = FNActorUtils::GetRootComponentFromDefaultObject(TSubclassOf<AActor>(BPGC));
	CHECK_MESSAGE(TEXT("Non-scene RootNodes[0] must be skipped in favour of the scene-component sibling"),
		Root != nullptr && Root == ExpectedTemplate);
}

N_TEST_HIGH(FNActorUtilsTests_GetRootComponentFromDefaultObject_InheritedSCS_ReturnsParentRoot,
	"NEXUS::UnitTests::NCore::FNActorUtils::GetRootComponentFromDefaultObject::InheritedSCS_ReturnsParentRoot",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Child BPGC has an empty SCS; parent BPGC's SCS defines a scene root. The walk must climb the BP chain and return
	// the parent's scene template — this is the inheritance case the pre-fix implementation could not resolve.
	USceneComponent* ExpectedTemplate = nullptr;
	UBlueprintGeneratedClass* ParentBPGC = NEXUS::UnitTests::NCore::FNActorUtilsHarness::MakeSyntheticBPGC(
		AActor::StaticClass(),
		[&ExpectedTemplate](USimpleConstructionScript* SCS)
		{
			USCS_Node* SceneNode = NEXUS::UnitTests::NCore::FNActorUtilsHarness::MakeNode(SCS, USceneComponent::StaticClass());
			SCS->AddNode(SceneNode);
			ExpectedTemplate = Cast<USceneComponent>(SceneNode->ComponentTemplate);
		});
	const TStrongObjectPtr<UBlueprintGeneratedClass> ParentGCGuard(ParentBPGC);

	UBlueprintGeneratedClass* ChildBPGC = NEXUS::UnitTests::NCore::FNActorUtilsHarness::MakeSyntheticBPGC(
		ParentBPGC,
		[](USimpleConstructionScript*) { /* empty — child does not override the root */ });
	const TStrongObjectPtr<UBlueprintGeneratedClass> ChildGCGuard(ChildBPGC);

	USceneComponent* Root = FNActorUtils::GetRootComponentFromDefaultObject(TSubclassOf<AActor>(ChildBPGC));
	CHECK_MESSAGE(TEXT("Child BPGC with empty SCS must inherit the parent BPGC's scene-root template"),
		Root != nullptr && Root == ExpectedTemplate);
}

#endif //WITH_TESTS

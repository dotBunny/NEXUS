// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldAssemblyEditorValidator.h"
#include "AssetRegistry/AssetData.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"
#include "Components/SceneComponent.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"
#include "Misc/DataValidation.h"

namespace NEXUS::UnitTests::NWorldAssembly::UNWorldAssemblyEditorValidatorHarness
{
	/**
	 * Populate the fixture world with a cell actor plus a host actor carrying a junction attached under a root of the
	 * given mobility, then run the world validator and report its result. A junction forces itself to Static mobility
	 * in its constructor, so the only variable that drives the cook-mismatch check is the parent's mobility.
	 * @param World The fixture world to populate.
	 * @param ParentMobility Mobility to apply to the junction's attach parent.
	 * @param OutNumErrors Receives the number of validation errors raised by the run.
	 * @return The validation result for the populated world.
	 */
	EDataValidationResult RunWorldValidationWithJunctionParent(UWorld* World, const EComponentMobility::Type ParentMobility, uint32& OutNumErrors)
	{
		// A cell actor must be present or ValidateWorldAsset never enters the junction sweep.
		World->SpawnActor<ANCellActor>();

		// Host actor whose root mobility we control. The root must start Static: AttachToComponent refuses a
		// Static child under a non-Static parent, so the junction can only be attached while the root is Static.
		AActor* Host = World->SpawnActor<AActor>();
		USceneComponent* HostRoot = NewObject<USceneComponent>(Host, TEXT("HostRoot"));
		HostRoot->SetMobility(EComponentMobility::Static);
		Host->SetRootComponent(HostRoot);
		HostRoot->RegisterComponent();

		UNCellJunctionComponent* Junction = NewObject<UNCellJunctionComponent>(Host, TEXT("Junction"));
		// Keep BeginPlay inert so the fixture exercises only registration + validation, not filler spawning.
		Junction->Details.Requirements = ENCellJunctionRequirements::AllowEmpty;
		Junction->SetupAttachment(HostRoot);
		Junction->RegisterComponent();

		// Reproduce the state a loaded level deserializes into: a Static junction still attached to a parent of
		// the mobility under test. The field is assigned directly rather than via SetMobility because SetMobility
		// propagates Movable down to children (and the editor path would detach the child) — guards that only
		// apply to live edits/attachment, not to the serialized attachment the cooker actually rejects.
		HostRoot->Mobility = ParentMobility;

		UNWorldAssemblyEditorValidator* Validator = NewObject<UNWorldAssemblyEditorValidator>();
		FDataValidationContext Context;
		const EDataValidationResult Result = Validator->ValidateLoadedAsset(FAssetData(World), World, Context);
		OutNumErrors = Context.GetNumErrors();
		return Result;
	}
}

N_TEST_HIGH(UNWorldAssemblyEditorValidatorTests_JunctionMobility_NonStaticParentIsInvalid,
	"NEXUS::UnitTests::NWorldAssembly::UNWorldAssemblyEditorValidator::JunctionMobility::NonStaticParentIsInvalid",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies a junction attached to a Movable parent fails validation; this is the condition that errors at cook time.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		uint32 NumErrors = 0;
		const EDataValidationResult Result = NEXUS::UnitTests::NWorldAssembly::UNWorldAssemblyEditorValidatorHarness::
			RunWorldValidationWithJunctionParent(World, EComponentMobility::Movable, NumErrors);

		CHECK_MESSAGE(TEXT("A junction under a non-Static parent should be invalid."), Result == EDataValidationResult::Invalid);
		CHECK_MESSAGE(TEXT("A junction under a non-Static parent should raise an error."), NumErrors > 0);
	});
}

N_TEST_HIGH(UNWorldAssemblyEditorValidatorTests_JunctionMobility_StaticParentIsValid,
	"NEXUS::UnitTests::NWorldAssembly::UNWorldAssemblyEditorValidator::JunctionMobility::StaticParentIsValid",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies a junction attached to a Static parent passes validation and raises no errors.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		uint32 NumErrors = 0;
		const EDataValidationResult Result = NEXUS::UnitTests::NWorldAssembly::UNWorldAssemblyEditorValidatorHarness::
			RunWorldValidationWithJunctionParent(World, EComponentMobility::Static, NumErrors);

		CHECK_MESSAGE(TEXT("A junction under a Static parent should be valid."), Result == EDataValidationResult::Valid);
		CHECK_MESSAGE(TEXT("A junction under a Static parent should raise no errors."), NumErrors == 0);
	});
}

#endif //WITH_TESTS

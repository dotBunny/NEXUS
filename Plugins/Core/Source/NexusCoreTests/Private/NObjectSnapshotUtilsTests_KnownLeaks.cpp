// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NCoreMinimal.h"
#include "Developer/NObjectSnapshotUtils.h"
#include "Engine/Texture2D.h"
#include "Macros/NTestMacros.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

namespace NEXUS::UnitTests::NCore::FNKnownLeaksHarness
{
	/** Wrap Object in a diff whose Added set holds only it, the shape RemoveKnownLeaks filters in place. */
	static FNObjectSnapshotDiff MakeDiffWithAdded(UObject* Object)
	{
		FNObjectSnapshotDiff Diff;

		FNObjectSnapshotEntry Entry;
		Entry.ObjectPtr = Object;
		Entry.Name = Object->GetFName().ToString();
		Entry.FullName = Object->GetFullName();

		Diff.Added.Add(Entry);
		Diff.AddedCount = 1;
		Diff.ChangeCount = 1;
		return Diff;
	}
}

N_TEST_HIGH(FNObjectSnapshotUtilsTests_KnownLeaks_RuntimeObjectIsReported,
	"NEXUS::UnitTests::NCore::FNObjectSnapshotUtils::KnownLeaks::RuntimeObjectIsReported",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The baseline the loaded-asset rule must not erode: an object a test allocated itself is exactly what the
	// leak check exists to catch, and nothing about it may be treated as known. Deliberately the same class the
	// loaded-asset test uses, so the only thing separating the two cases is where the object came from.
	using namespace NEXUS::UnitTests::NCore::FNKnownLeaksHarness;

	const TStrongObjectPtr<UObject> Runtime(NewObject<UTexture2D>(GetTransientPackage()));
	CHECK_FALSE_MESSAGE(TEXT("A NewObject'd object must not carry RF_WasLoaded."),
		Runtime->HasAnyFlags(RF_WasLoaded))

	FNObjectSnapshotDiff Diff = MakeDiffWithAdded(Runtime.Get());
	FNObjectSnapshotUtils::RemoveKnownLeaks(Diff);

	CHECK_EQUALS("A runtime-allocated object must still be reported as a leak.", Diff.AddedCount, 1)
}

N_TEST_HIGH(FNObjectSnapshotUtilsTests_KnownLeaks_LoadedAssetIsNotReported,
	"NEXUS::UnitTests::NCore::FNObjectSnapshotUtils::KnownLeaks::LoadedAssetIsNotReported",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A test that touches an asset for the first time pays to load it, and the asset stays resident afterwards by
	// design — so it appears in the post-test snapshot and would otherwise be reported. Whichever test loads it
	// first would fail while every later one passed, which is the order-dependence that gives the false positive
	// away. Assets are recognised by RF_WasLoaded, the flag the engine stamps on anything it serialised off disk.
	using namespace NEXUS::UnitTests::NCore::FNKnownLeaksHarness;

	// Any always-present engine asset works; nothing here depends on it being a texture.
	const TStrongObjectPtr<UObject> Asset(LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/DefaultTexture")));
	if (!Asset.IsValid())
	{
		ADD_ERROR("Expected to be able to load /Engine/EngineResources/DefaultTexture.");
		return;
	}

	CHECK_MESSAGE(TEXT("An asset serialised off disk must carry RF_WasLoaded."),
		Asset->HasAnyFlags(RF_WasLoaded))

	FNObjectSnapshotDiff Diff = MakeDiffWithAdded(Asset.Get());
	FNObjectSnapshotUtils::RemoveKnownLeaks(Diff);

	CHECK_EQUALS("A loaded asset must not be reported as a leak.", Diff.AddedCount, 0)
	CHECK_EQUALS("Dropping the entry must keep the churn total consistent.", Diff.ChangeCount, 0)
}

N_TEST_HIGH(FNObjectSnapshotUtilsTests_KnownLeaks_LoadedAssetPackageIsNotReported,
	"NEXUS::UnitTests::NCore::FNObjectSnapshotUtils::KnownLeaks::LoadedAssetPackageIsNotReported",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Loading an asset brings its UPackage in alongside it, and the package is reported separately from the asset.
	// It is loaded too, so the same rule has to cover it — otherwise suppressing the asset alone still leaves the
	// test red on the package.
	using namespace NEXUS::UnitTests::NCore::FNKnownLeaksHarness;

	const TStrongObjectPtr<UObject> Asset(LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/DefaultTexture")));
	if (!Asset.IsValid())
	{
		ADD_ERROR("Expected to be able to load /Engine/EngineResources/DefaultTexture.");
		return;
	}

	UPackage* Package = Asset->GetPackage();
	if (Package == nullptr)
	{
		ADD_ERROR("Expected the loaded asset to report a package.");
		return;
	}

	FNObjectSnapshotDiff Diff = MakeDiffWithAdded(Package);
	FNObjectSnapshotUtils::RemoveKnownLeaks(Diff);

	CHECK_EQUALS("The package of a loaded asset must not be reported as a leak.", Diff.AddedCount, 0)
}

#endif //WITH_TESTS

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NCoreMinimal.h"
#include "NObjectSnapshotTestTypes.h"
#include "Developer/NObjectSnapshotUtils.h"
#include "Macros/NTestMacros.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

namespace NEXUS::UnitTests::NCore::FNSnapshotDetectionHarness
{
	/**
	 * @return Whether any entry in the diff's Added set is an object of Class.
	 * @note Entries whose object has since been collected report a null ObjectPtr and never match, which is what
	 *       lets the "was collected" control assert on absence without dereferencing a stale pointer.
	 */
	static bool AddedContainsClass(const FNObjectSnapshotDiff& Diff, const UClass* Class)
	{
		for (const FNObjectSnapshotEntry& Entry : Diff.Added)
		{
			if (const UObject* Object = Entry.ObjectPtr.Get(); Object != nullptr && Object->IsA(Class))
			{
				return true;
			}
		}
		return false;
	}

	/** @return Whether any entry in the diff's Added set is exactly Object. */
	static bool AddedContains(const FNObjectSnapshotDiff& Diff, const UObject* Object)
	{
		for (const FNObjectSnapshotEntry& Entry : Diff.Added)
		{
			if (Entry.ObjectPtr.Get() == Object)
			{
				return true;
			}
		}
		return false;
	}
}

N_TEST_CRITICAL(FNObjectSnapshotUtilsTests_Detection_ReportsAnObjectThatSurvivesCollection,
	"NEXUS::UnitTests::NCore::FNObjectSnapshotUtils::Detection::ReportsAnObjectThatSurvivesCollection",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The load-bearing test for the whole leak check: plant an actual leak and prove the detector reports it.
	// Everything else in these files asserts that something is *not* reported, and a detector that reported
	// nothing at all would satisfy every one of them. This is what stops that from passing silently.
	//
	// The sequence below is the one FNTestUtils::WorldTestChecked runs internally — collect, snapshot, do the work,
	// collect again, snapshot, diff — reproduced here so the assertion can land on the diff rather than on the
	// ADD_ERROR that WorldTestChecked would raise, which would fail this test rather than prove it.
	using namespace NEXUS::UnitTests::NCore::FNSnapshotDetectionHarness;

	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	const FNObjectSnapshot PreSnapshot = FNObjectSnapshotUtils::Snapshot();

	// The leak. A strong reference held across the collection pass is exactly the shape of the bug this check
	// exists to find: something still holding the object after the scope that owned it has gone.
	const TStrongObjectPtr<UNObjectSnapshotTestObject> Leaked(NewObject<UNObjectSnapshotTestObject>(GetTransientPackage()));

	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	const FNObjectSnapshot PostSnapshot = FNObjectSnapshotUtils::Snapshot();

	CHECK_MESSAGE(TEXT("The leaked object must survive the collection pass, or the fixture proves nothing."),
		Leaked.IsValid())

	// Filtered exactly as the leak check filters, so this also pins that the known-leak rules — the loaded-asset
	// one in particular — do not swallow a genuine runtime leak on their way past.
	FNObjectSnapshotDiff Diff = FNObjectSnapshotUtils::Diff(PreSnapshot, PostSnapshot, true);

	CHECK_MESSAGE(TEXT("A leaked object must be reported in the diff's Added set."),
		AddedContains(Diff, Leaked.Get()))
	CHECK_MESSAGE(TEXT("The reported leak must survive the known-leak filter."),
		Diff.AddedCount > 0)
}

N_TEST_CRITICAL(FNObjectSnapshotUtilsTests_Detection_DoesNotReportACollectedObject,
	"NEXUS::UnitTests::NCore::FNObjectSnapshotUtils::Detection::DoesNotReportACollectedObject",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The other half of the pair: an object that is created and then properly released must not be reported. A
	// detector that flagged everything a test allocated would catch the leak above while being useless in practice.
	using namespace NEXUS::UnitTests::NCore::FNSnapshotDetectionHarness;

	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	const FNObjectSnapshot PreSnapshot = FNObjectSnapshotUtils::Snapshot();

	TWeakObjectPtr<UNObjectSnapshotTestObject> Temporary;
	{
		// Deliberately no strong reference kept past this scope.
		Temporary = NewObject<UNObjectSnapshotTestObject>(GetTransientPackage());
		CHECK_MESSAGE(TEXT("The fixture object must exist before the collection pass, or the test is vacuous."),
			Temporary.IsValid())
	}

	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	const FNObjectSnapshot PostSnapshot = FNObjectSnapshotUtils::Snapshot();

	CHECK_FALSE_MESSAGE(TEXT("An unreferenced object must be collected rather than left to be reported."),
		Temporary.IsValid())

	FNObjectSnapshotDiff Diff = FNObjectSnapshotUtils::Diff(PreSnapshot, PostSnapshot, true);

	CHECK_FALSE_MESSAGE(TEXT("A collected object must not be reported as a leak."),
		AddedContainsClass(Diff, UNObjectSnapshotTestObject::StaticClass()))
}

#endif //WITH_TESTS

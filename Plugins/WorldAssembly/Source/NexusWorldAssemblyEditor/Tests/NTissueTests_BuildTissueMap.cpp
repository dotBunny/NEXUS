// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Cell/NCell.h"
#include "Cell/NTissue.h"
#include "GameplayTagContainer.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNTissueBuildTissueMapHarness
{
	// The built-in tag symbols are declared without an export macro, so they can't be linked from the editor
	// module; request them by their registered name instead (matches the strings in NWorldAssemblyGameplayTags.cpp).
	static FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	static UNCell* MakeCell()
	{
		return NewObject<UNCell>(GetTransientPackage());
	}

	static UNTissue* MakeTissue()
	{
		return NewObject<UNTissue>(GetTransientPackage());
	}

	/** Append a cell entry to a tissue, optionally carrying an assembly tag. */
	static void AddCell(UNTissue* Tissue, UNCell* Cell, const FGameplayTag& AssemblyTag = FGameplayTag())
	{
		FNTissueEntry Entry;
		Entry.Cell = Cell;
		if (AssemblyTag.IsValid())
		{
			Entry.AssemblyTags.AddTag(AssemblyTag);
		}
		Tissue->Cells.Add(Entry);
	}
}

N_TEST_CRITICAL(FNTissueTests_BuildTissueMap_FlattensDistinctCells,
	"NEXUS::UnitTests::NWorldAssembly::UNTissue::BuildTissueMap::FlattensDistinctCells",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the base case: every distinct cell in a tissue ends up in the flattened map exactly once.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNTissueBuildTissueMapHarness;

	UNTissue* Tissue = MakeTissue();
	UNCell* CellA = MakeCell();
	UNCell* CellB = MakeCell();
	AddCell(Tissue, CellA);
	AddCell(Tissue, CellB);

	TMap<TObjectPtr<UNCell>, FNTissueEntry> CellMap;
	FNTissueTagGroups TagGroups;
	TArray<UNTissue*> Processed;
	UNTissue::BuildTissueMap(Tissue, CellMap, TagGroups, Processed);

	CHECK_EQUALS("Two distinct cells should produce two map entries.", CellMap.Num(), 2)
	CHECK_MESSAGE(TEXT("Cell A should be present in the flattened map."), CellMap.Contains(CellA))
	CHECK_MESSAGE(TEXT("Cell B should be present in the flattened map."), CellMap.Contains(CellB))
}

N_TEST_HIGH(FNTissueTests_BuildTissueMap_SkipsNullCellEntries,
	"NEXUS::UnitTests::NWorldAssembly::UNTissue::BuildTissueMap::SkipsNullCellEntries",
	N_TEST_CONTEXT_ANYWHERE)
{
	// An entry whose Cell reference is unset must be skipped rather than inserting a null key.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNTissueBuildTissueMapHarness;

	UNTissue* Tissue = MakeTissue();
	UNCell* CellA = MakeCell();
	AddCell(Tissue, CellA);
	Tissue->Cells.Add(FNTissueEntry()); // Entry with an empty Cell soft pointer.

	TMap<TObjectPtr<UNCell>, FNTissueEntry> CellMap;
	FNTissueTagGroups TagGroups;
	TArray<UNTissue*> Processed;
	UNTissue::BuildTissueMap(Tissue, CellMap, TagGroups, Processed);

	CHECK_EQUALS("Only the one valid cell should be mapped; the null entry is skipped.", CellMap.Num(), 1)
	CHECK_FALSE_MESSAGE(TEXT("A null cell key must never be inserted."), CellMap.Contains(nullptr))
}

N_TEST_HIGH(FNTissueTests_BuildTissueMap_MergesDuplicateCellTags,
	"NEXUS::UnitTests::NWorldAssembly::UNTissue::BuildTissueMap::MergesDuplicateCellTags",
	N_TEST_CONTEXT_ANYWHERE)
{
	// When the same cell appears twice, the second entry's assembly tags are merged into the first
	// rather than producing a duplicate map entry.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNTissueBuildTissueMapHarness;

	UNTissue* Tissue = MakeTissue();
	UNCell* Cell = MakeCell();
	AddCell(Tissue, Cell, Tag(TEXT("NEXUS.WorldAssembly.Behavior.Starter")));
	AddCell(Tissue, Cell, Tag(TEXT("NEXUS.WorldAssembly.Behavior.Finisher")));

	TMap<TObjectPtr<UNCell>, FNTissueEntry> CellMap;
	FNTissueTagGroups TagGroups;
	TArray<UNTissue*> Processed;
	UNTissue::BuildTissueMap(Tissue, CellMap, TagGroups, Processed);

	CHECK_EQUALS("A repeated cell should collapse to a single map entry.", CellMap.Num(), 1)

	const FNTissueEntry* Entry = CellMap.Find(Cell);
	if (Entry == nullptr)
	{
		ADD_ERROR("Expected the cell to be present in the map.");
		return;
	}
	CHECK_MESSAGE(TEXT("Merged entry should retain the first entry's tag."),
		Entry->AssemblyTags.HasTag(Tag(TEXT("NEXUS.WorldAssembly.Behavior.Starter"))))
	CHECK_MESSAGE(TEXT("Merged entry should also carry the duplicate entry's tag."),
		Entry->AssemblyTags.HasTag(Tag(TEXT("NEXUS.WorldAssembly.Behavior.Finisher"))))
}

N_TEST_HIGH(FNTissueTests_BuildTissueMap_MergesAdditionalTissue,
	"NEXUS::UnitTests::NWorldAssembly::UNTissue::BuildTissueMap::MergesAdditionalTissue",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Cells reachable through AdditionalTissue should be folded into the same flattened map.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNTissueBuildTissueMapHarness;

	UNTissue* Root = MakeTissue();
	UNTissue* Additional = MakeTissue();
	UNCell* RootCell = MakeCell();
	UNCell* AdditionalCell = MakeCell();
	AddCell(Root, RootCell);
	AddCell(Additional, AdditionalCell);
	Root->AdditionalTissue.Add(Additional);

	TMap<TObjectPtr<UNCell>, FNTissueEntry> CellMap;
	FNTissueTagGroups TagGroups;
	TArray<UNTissue*> Processed;
	UNTissue::BuildTissueMap(Root, CellMap, TagGroups, Processed);

	CHECK_EQUALS("Cells from the root and the additional tissue should both be mapped.", CellMap.Num(), 2)
	CHECK_MESSAGE(TEXT("Root cell should be present."), CellMap.Contains(RootCell))
	CHECK_MESSAGE(TEXT("Additional-tissue cell should be present."), CellMap.Contains(AdditionalCell))
}

N_TEST_CRITICAL(FNTissueTests_BuildTissueMap_TerminatesOnCycle,
	"NEXUS::UnitTests::NWorldAssembly::UNTissue::BuildTissueMap::TerminatesOnCycle",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A mutual reference (A -> B -> A) must not recurse forever; the processed-set guard breaks the cycle
	// while still folding both tissues' cells into the map.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNTissueBuildTissueMapHarness;

	UNTissue* TissueA = MakeTissue();
	UNTissue* TissueB = MakeTissue();
	UNCell* CellA = MakeCell();
	UNCell* CellB = MakeCell();
	AddCell(TissueA, CellA);
	AddCell(TissueB, CellB);
	TissueA->AdditionalTissue.Add(TissueB);
	TissueB->AdditionalTissue.Add(TissueA);

	TMap<TObjectPtr<UNCell>, FNTissueEntry> CellMap;
	FNTissueTagGroups TagGroups;
	TArray<UNTissue*> Processed;
	UNTissue::BuildTissueMap(TissueA, CellMap, TagGroups, Processed);

	// Reaching this line at all proves the recursion terminated.
	CHECK_EQUALS("A cyclic tissue pair should still map both cells exactly once.", CellMap.Num(), 2)
	CHECK_MESSAGE(TEXT("Cell A should be present despite the cycle."), CellMap.Contains(CellA))
	CHECK_MESSAGE(TEXT("Cell B should be present despite the cycle."), CellMap.Contains(CellB))
}

N_TEST_MEDIUM(FNTissueTests_BuildTissueMap_AccumulatesTagGroups,
	"NEXUS::UnitTests::NWorldAssembly::UNTissue::BuildTissueMap::AccumulatesTagGroups",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The output tag groups should carry the framework baseline (EnsureBaseTags) plus any tags contributed
	// by the visited tissues (AppendTags).
	using namespace NEXUS::UnitTests::NWorldAssembly::FNTissueBuildTissueMapHarness;

	UNTissue* Tissue = MakeTissue();
	UNCell* Cell = MakeCell();
	AddCell(Tissue, Cell);
	// Seed a distinctive tag into the tissue's Unique set so we can prove AppendTags propagated it.
	Tissue->TagGroups.AppendUniqueTags(FGameplayTagContainer(Tag(TEXT("NEXUS.WorldAssembly.Behavior.RequiredAny"))));

	TMap<TObjectPtr<UNCell>, FNTissueEntry> CellMap;
	FNTissueTagGroups TagGroups;
	TArray<UNTissue*> Processed;
	UNTissue::BuildTissueMap(Tissue, CellMap, TagGroups, Processed);

	CHECK_MESSAGE(TEXT("Baseline built-in Unique tag should be present after EnsureBaseTags."),
		TagGroups.HasAnyUniqueTags(FGameplayTagContainer(Tag(TEXT("NEXUS.WorldAssembly.Behavior.Unique")))))
	CHECK_MESSAGE(TEXT("Baseline built-in RequiredAny tag should be present after EnsureBaseTags."),
		TagGroups.HasAnyRequiredAnyTags(FGameplayTagContainer(Tag(TEXT("NEXUS.WorldAssembly.Behavior.RequiredAny")))))
	CHECK_MESSAGE(TEXT("The tissue's custom unique tag should have been merged into the output."),
		TagGroups.HasAnyUniqueTags(FGameplayTagContainer(Tag(TEXT("NEXUS.WorldAssembly.Behavior.RequiredAny")))))
}

#endif //WITH_TESTS

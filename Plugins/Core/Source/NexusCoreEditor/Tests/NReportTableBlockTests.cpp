// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Developer/Report/NReport.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNReportTableBlockTests_Initialize_HeaderRow_Markdown,
	"NEXUS::UnitTests::NCore::FNReportTableBlock::Initialize::HeaderRow_Markdown",
	N_TEST_CONTEXT_ANYWHERE)
{
	FNReport Report;
	const int32 Ticket = Report.CreateTableBlock(0, 0);
	FNReportTableBlock* Block = Report.GetTableBlock(Ticket);
	if (Block == nullptr)
	{
		ADD_ERROR("Newly created table block was not found in the report.");
		return;
	}
	Block->Initialize({ TEXT("Name"), TEXT("Score") });
	Block->AddRow({ TEXT("Alice"), TEXT("42") });

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::Markdown);
	CHECK_EQUALS("Markdown table should emit header row, divider, and one data row.",
		Lines.Num(), 3);
	if (Lines.Num() < 3) return;
	CHECK_EQUALS("Markdown header row should be pipe-separated.",
		Lines[0], FString(TEXT("| Name | Score |")));
	CHECK_EQUALS("Markdown divider row should follow the header.",
		Lines[1], FString(TEXT("| --- | --- |")));
	CHECK_EQUALS("Markdown data row should be pipe-separated.",
		Lines[2], FString(TEXT("| Alice | 42 |")));
}

N_TEST_HIGH(FNReportTableBlockTests_Initialize_HeaderRow_PlainText,
	"NEXUS::UnitTests::NCore::FNReportTableBlock::Initialize::HeaderRow_PlainText",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies plain text rendering pads each cell to the widest entry in its column.
	FNReport Report;
	const int32 Ticket = Report.CreateTableBlock(0, 0);
	FNReportTableBlock* Block = Report.GetTableBlock(Ticket);
	if (Block == nullptr)
	{
		ADD_ERROR("Newly created table block was not found in the report.");
		return;
	}
	Block->Initialize({ TEXT("Name"), TEXT("Score") });
	Block->AddRow({ TEXT("Alice"), TEXT("42") });

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::PlainText);
	CHECK_EQUALS("Plain text table should emit header, divider, and one data row.",
		Lines.Num(), 3);
	if (Lines.Num() < 3) return;
	// Column widths: Name col = max(Len("Name")=4, Len("Alice")=5) = 5; Score col = max(5, 2) = 5.
	CHECK_EQUALS("Plain text header should pad to the widest cell in each column.",
		Lines[0], FString(TEXT("Name   Score")));
	CHECK_EQUALS("Plain text divider should match each column's width.",
		Lines[1], FString(TEXT("-----  -----")));
	CHECK_EQUALS("Plain text data row should pad to the widest cell in each column.",
		Lines[2], FString(TEXT("Alice  42   ")));
}

N_TEST_HIGH(FNReportTableBlockTests_Initialize_ColumnCount_NoHeader,
	"NEXUS::UnitTests::NCore::FNReportTableBlock::Initialize::ColumnCount_NoHeader",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that a column-count Initialize emits no header section in plain text.
	FNReport Report;
	const int32 Ticket = Report.CreateTableBlock(0, 0);
	FNReportTableBlock* Block = Report.GetTableBlock(Ticket);
	if (Block == nullptr)
	{
		ADD_ERROR("Newly created table block was not found in the report.");
		return;
	}
	Block->Initialize(2);
	Block->AddRow({ TEXT("a"), TEXT("b") });

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::PlainText);
	CHECK_EQUALS("Plain text without a header row should emit only the data rows.",
		Lines.Num(), 1);
	if (Lines.Num() < 1) return;
	CHECK_EQUALS("Plain text data row should be padded to column widths.",
		Lines[0], FString(TEXT("a  b")));
}

N_TEST_HIGH(FNReportTableBlockTests_AddRow_DiscardsExtraCells,
	"NEXUS::UnitTests::NCore::FNReportTableBlock::AddRow::DiscardsExtraCells",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that cells beyond the column count are dropped at insertion time.
	FNReport Report;
	const int32 Ticket = Report.CreateTableBlock(0, 0);
	FNReportTableBlock* Block = Report.GetTableBlock(Ticket);
	if (Block == nullptr)
	{
		ADD_ERROR("Newly created table block was not found in the report.");
		return;
	}
	Block->Initialize(2);
	
	AddExpectedMessage(TEXT("Attempting to add data row exceeding"), ELogVerbosity::Warning);
	Block->AddRow({ TEXT("a"), TEXT("b"), TEXT("c") });

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::Markdown);
	CHECK_EQUALS("Markdown should emit empty header row, divider, and one data row.",
		Lines.Num(), 3);
	if (Lines.Num() < 3) return;
	CHECK_EQUALS("Data row should contain exactly the first ColumnCount cells.",
		Lines[2], FString(TEXT("| a | b |")));
	CHECK_FALSE_MESSAGE(TEXT("Discarded third cell value 'c' should not appear in the row."),
		Lines[2].Contains(TEXT("c")));
}

N_TEST_HIGH(FNReportTableBlockTests_AddRow_ShortRowPadsEmpty,
	"NEXUS::UnitTests::NCore::FNReportTableBlock::AddRow::ShortRowPadsEmpty",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that rendering pads short rows with empty cells out to ColumnCount.
	FNReport Report;
	const int32 Ticket = Report.CreateTableBlock(0, 0);
	FNReportTableBlock* Block = Report.GetTableBlock(Ticket);
	if (Block == nullptr)
	{
		ADD_ERROR("Newly created table block was not found in the report.");
		return;
	}
	Block->Initialize({ TEXT("A"), TEXT("B"), TEXT("C") });
	Block->AddRow({ TEXT("1") });

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::Markdown);
	CHECK_EQUALS("Markdown should emit header, divider, and one data row.", Lines.Num(), 3);
	if (Lines.Num() < 3) return;
	CHECK_EQUALS("Missing trailing cells should render as empty pipe-separated entries.",
		Lines[2], FString(TEXT("| 1 |  |  |")));
}

N_TEST_HIGH(FNReportTableBlockTests_Render_Markdown_MultipleRows,
	"NEXUS::UnitTests::NCore::FNReportTableBlock::Render::Markdown_MultipleRows",
	N_TEST_CONTEXT_ANYWHERE)
{
	FNReport Report;
	const int32 Ticket = Report.CreateTableBlock(0, 0);
	FNReportTableBlock* Block = Report.GetTableBlock(Ticket);
	if (Block == nullptr)
	{
		ADD_ERROR("Newly created table block was not found in the report.");
		return;
	}
	Block->Initialize({ TEXT("K"), TEXT("V") });
	Block->AddRow({ TEXT("k1"), TEXT("v1") });
	Block->AddRow({ TEXT("k2"), TEXT("v2") });
	Block->AddRow({ TEXT("k3"), TEXT("v3") });

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::Markdown);
	CHECK_EQUALS("Markdown should emit header, divider, and one line per data row.",
		Lines.Num(), 5);
	if (Lines.Num() < 5) return;
	CHECK_EQUALS("First data row should appear after the divider.",
		Lines[2], FString(TEXT("| k1 | v1 |")));
	CHECK_EQUALS("Second data row should preserve insertion order.",
		Lines[3], FString(TEXT("| k2 | v2 |")));
	CHECK_EQUALS("Third data row should preserve insertion order.",
		Lines[4], FString(TEXT("| k3 | v3 |")));
}

N_TEST_MEDIUM(FNReportTableBlockTests_Initialize_Reset,
	"NEXUS::UnitTests::NCore::FNReportTableBlock::Initialize::Reset",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that re-initializing the table clears any previously added rows and header.
	FNReport Report;
	const int32 Ticket = Report.CreateTableBlock(0, 0);
	FNReportTableBlock* Block = Report.GetTableBlock(Ticket);
	if (Block == nullptr)
	{
		ADD_ERROR("Newly created table block was not found in the report.");
		return;
	}
	Block->Initialize({ TEXT("Old"), TEXT("Header") });
	Block->AddRow({ TEXT("stale"), TEXT("data") });
	Block->Initialize({ TEXT("New") });
	Block->AddRow({ TEXT("fresh") });

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::Markdown);
	CHECK_EQUALS("Re-initialized table should emit only the new header, divider, and row.",
		Lines.Num(), 3);
	if (Lines.Num() < 3) return;
	CHECK_EQUALS("Re-initialized header row should reflect the most recent Initialize call.",
		Lines[0], FString(TEXT("| New |")));
	CHECK_EQUALS("Re-initialized data row should reflect rows added after the new Initialize.",
		Lines[2], FString(TEXT("| fresh |")));
}

#endif //WITH_TESTS

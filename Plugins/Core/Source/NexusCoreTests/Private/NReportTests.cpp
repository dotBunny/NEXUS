// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Developer/NReport.h"
#include "HAL/PlatformFileManager.h"
#include "Macros/NTestMacros.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNReportTests_CreateContentBlock_AscendingTickets,
	"NEXUS::UnitTests::NCore::FNReport::CreateContentBlock::AscendingTickets",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that the per-report ticket counter starts at 1 and increments per allocation.
	FNReport Report;
	const int32 First = Report.CreateContentBlock(0, 0);
	const int32 Second = Report.CreateContentBlock(0, 0);
	const int32 Third = Report.CreateContentBlock(0, 0);

	CHECK_EQUALS("First content block ticket should be 1.", First, 1);
	CHECK_EQUALS("Second content block ticket should be 2.", Second, 2);
	CHECK_EQUALS("Third content block ticket should be 3.", Third, 3);
}

N_TEST_HIGH(FNReportTests_CreateBlocks_ShareTicketCounter,
	"NEXUS::UnitTests::NCore::FNReport::CreateBlocks::ShareTicketCounter",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that content and table blocks draw from a single shared ticket counter.
	FNReport Report;
	const int32 ContentTicket = Report.CreateContentBlock(0, 0);
	const int32 TableTicket = Report.CreateTableBlock(0, 0);
	const int32 NextContent = Report.CreateContentBlock(0, 0);

	CHECK_EQUALS("First created block (content) should receive ticket 1.", ContentTicket, 1);
	CHECK_EQUALS("Second created block (table) should receive ticket 2.", TableTicket, 2);
	CHECK_EQUALS("Third created block should continue the shared sequence at ticket 3.",
		NextContent, 3);
}

N_TEST_HIGH(FNReportTests_GetContentBlock_ReturnsCreatedBlock,
	"NEXUS::UnitTests::NCore::FNReport::GetContentBlock::ReturnsCreatedBlock",
	N_TEST_CONTEXT_ANYWHERE)
{
	FNReport Report;
	const int32 Ticket = Report.CreateContentBlock(0, 0);
	FNReportContentBlock* Block = Report.GetContentBlock(Ticket);

	if (Block == nullptr)
	{
		ADD_ERROR("GetContentBlock should return a valid pointer immediately after creation.");
		return;
	}
	CHECK_EQUALS("Returned block should report the ticket it was created with.",
		Block->GetTicket(), Ticket);
}

N_TEST_HIGH(FNReportTests_GetTableBlock_ReturnsCreatedBlock,
	"NEXUS::UnitTests::NCore::FNReport::GetTableBlock::ReturnsCreatedBlock",
	N_TEST_CONTEXT_ANYWHERE)
{
	FNReport Report;
	const int32 Ticket = Report.CreateTableBlock(0, 0);
	FNReportTableBlock* Block = Report.GetTableBlock(Ticket);

	if (Block == nullptr)
	{
		ADD_ERROR("GetTableBlock should return a valid pointer immediately after creation.");
		return;
	}
	CHECK_EQUALS("Returned block should report the ticket it was created with.",
		Block->GetTicket(), Ticket);
}

N_TEST_HIGH(FNReportTests_GetContentBlock_NullForInvalidTicket,
	"NEXUS::UnitTests::NCore::FNReport::GetContentBlock::NullForInvalidTicket",
	N_TEST_CONTEXT_ANYWHERE)
{
	FNReport Report;
	CHECK_MESSAGE(TEXT("GetContentBlock should return nullptr for an unknown ticket."),
		Report.GetContentBlock(99) == nullptr);
}

N_TEST_HIGH(FNReportTests_GetContentBlock_NullForTableTicket,
	"NEXUS::UnitTests::NCore::FNReport::GetContentBlock::NullForTableTicket",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that ContentBlocks and TableBlocks live in separate maps keyed by ticket.
	FNReport Report;
	const int32 TableTicket = Report.CreateTableBlock(0, 0);
	CHECK_MESSAGE(TEXT("GetContentBlock should return nullptr when the ticket belongs to a table block."),
		Report.GetContentBlock(TableTicket) == nullptr);
}

N_TEST_HIGH(FNReportTests_GetTableBlock_NullForContentTicket,
	"NEXUS::UnitTests::NCore::FNReport::GetTableBlock::NullForContentTicket",
	N_TEST_CONTEXT_ANYWHERE)
{
	FNReport Report;
	const int32 ContentTicket = Report.CreateContentBlock(0, 0);
	CHECK_MESSAGE(TEXT("GetTableBlock should return nullptr when the ticket belongs to a content block."),
		Report.GetTableBlock(ContentTicket) == nullptr);
}

N_TEST_HIGH(FNReportTests_GetReportLines_EmptyReport,
	"NEXUS::UnitTests::NCore::FNReport::GetReportLines::EmptyReport",
	N_TEST_CONTEXT_ANYWHERE)
{
	FNReport Report;
	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::Markdown);
	CHECK_EQUALS("An empty report should produce no output lines.", Lines.Num(), 0);
}

N_TEST_HIGH(FNReportTests_GetReportLines_OrderedByPriority,
	"NEXUS::UnitTests::NCore::FNReport::GetReportLines::OrderedByPriority",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that top-level blocks render in ascending priority order regardless of insertion order.
	FNReport Report;
	const int32 High = Report.CreateContentBlock(0, 5);
	const int32 Low = Report.CreateContentBlock(0, 1);
	const int32 Mid = Report.CreateContentBlock(0, 3);

	FNReportContentBlock* HighBlock = Report.GetContentBlock(High);
	FNReportContentBlock* LowBlock = Report.GetContentBlock(Low);
	FNReportContentBlock* MidBlock = Report.GetContentBlock(Mid);
	if (HighBlock == nullptr || LowBlock == nullptr || MidBlock == nullptr)
	{
		ADD_ERROR("Could not find newly created blocks in the report.");
		return;
	}
	HighBlock->SetHeading(TEXT("High"));
	LowBlock->SetHeading(TEXT("Low"));
	MidBlock->SetHeading(TEXT("Mid"));

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::Markdown);
	// Each markdown heading emits a leading blank, the heading line, and a trailing blank — 3 lines per block.
	CHECK_EQUALS("Three top-level blocks should produce nine markdown lines.", Lines.Num(), 9);
	if (Lines.Num() < 9) return;
	CHECK_EQUALS("Lowest priority block should render first.", Lines[1], FString(TEXT("# Low")));
	CHECK_EQUALS("Middle priority block should render second.", Lines[4], FString(TEXT("# Mid")));
	CHECK_EQUALS("Highest priority block should render last.", Lines[7], FString(TEXT("# High")));
}

N_TEST_HIGH(FNReportTests_GetReportLines_MixedBlockTypes,
	"NEXUS::UnitTests::NCore::FNReport::GetReportLines::MixedBlockTypes",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that top-level rendering dispatches to both content and table block renderers.
	FNReport Report;
	const int32 ContentTicket = Report.CreateContentBlock(0, 0);
	const int32 TableTicket = Report.CreateTableBlock(0, 1);
	FNReportContentBlock* Content = Report.GetContentBlock(ContentTicket);
	FNReportTableBlock* Table = Report.GetTableBlock(TableTicket);
	if (Content == nullptr || Table == nullptr)
	{
		ADD_ERROR("Could not find newly created blocks in the report.");
		return;
	}
	Content->AddLine(TEXT("ContentLine"));
	Table->Initialize({ TEXT("Col") });
	Table->AddRow({ TEXT("Cell") });

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::Markdown);
	CHECK_MESSAGE(TEXT("Content block output should be present."),
		Lines.Contains(FString(TEXT("ContentLine"))));
	CHECK_MESSAGE(TEXT("Table block header should be present."),
		Lines.Contains(FString(TEXT("| Col |"))));
	CHECK_MESSAGE(TEXT("Table block data row should be present."),
		Lines.Contains(FString(TEXT("| Cell |"))));
}

N_TEST_HIGH(FNReportTests_OutputToFile_WritesContent,
	"NEXUS::UnitTests::NCore::FNReport::OutputToFile::WritesContent",
	N_TEST_CONTEXT_ANYWHERE)
{
	FNReport Report;
	const int32 Ticket = Report.CreateContentBlock(0, 0);
	FNReportContentBlock* Block = Report.GetContentBlock(Ticket);
	if (Block == nullptr)
	{
		ADD_ERROR("Newly created content block was not found in the report.");
		return;
	}
	Block->SetHeading(TEXT("WrittenReport"));

	const FString FilePath = FPaths::Combine(FPaths::ProjectIntermediateDir(),
		TEXT("NReportTests"), FGuid::NewGuid().ToString() + TEXT(".md"));
	Report.OutputToFile(FilePath, ENReportOutputFormat::Markdown);

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.FileExists(*FilePath))
	{
		ADD_ERROR(FString::Printf(TEXT("OutputToFile did not produce a file at %s."), *FilePath));
		return;
	}

	TArray<FString> Lines;
	const bool bLoaded = FFileHelper::LoadFileToStringArray(Lines, *FilePath);
	PlatformFile.DeleteFile(*FilePath);

	if (!bLoaded)
	{
		ADD_ERROR("Could not read back the file written by OutputToFile.");
		return;
	}
	CHECK_MESSAGE(TEXT("Written file should contain the rendered markdown heading line."),
		Lines.Contains(FString(TEXT("# WrittenReport"))));
}

N_TEST_HIGH(FNReportTests_OutputToFile_CreatesMissingDirectory,
	"NEXUS::UnitTests::NCore::FNReport::OutputToFile::CreatesMissingDirectory",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that OutputToFile creates the full intermediate directory tree on demand.
	FNReport Report;
	Report.CreateContentBlock(0, 0);

	const FString DirectoryName = FGuid::NewGuid().ToString();
	const FString Directory = FPaths::Combine(FPaths::ProjectIntermediateDir(),
		TEXT("NReportTests"), DirectoryName);
	const FString FilePath = FPaths::Combine(Directory, TEXT("report.md"));

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.DirectoryExists(*Directory))
	{
		ADD_ERROR("Test directory unexpectedly already exists; GUID collision?");
		return;
	}

	Report.OutputToFile(FilePath, ENReportOutputFormat::PlainText);

	const bool bDirectoryCreated = PlatformFile.DirectoryExists(*Directory);
	if (PlatformFile.FileExists(*FilePath))
	{
		PlatformFile.DeleteFile(*FilePath);
	}
	PlatformFile.DeleteDirectory(*Directory);

	CHECK_MESSAGE(TEXT("OutputToFile should create any missing intermediate directories."),
		bDirectoryCreated);
}

#endif //WITH_TESTS

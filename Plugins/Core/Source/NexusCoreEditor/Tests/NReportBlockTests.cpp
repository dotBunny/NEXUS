// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Developer/NReport.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNReportBlockTests_Equality_SameTicket,
	"NEXUS::UnitTests::NCore::FNReportBlock::Equality::SameTicket",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNReportBlock A(7);
	const FNReportBlock B(7);
	CHECK_MESSAGE(TEXT("Blocks with the same ticket should compare equal."), A == B);
}

N_TEST_HIGH(FNReportBlockTests_Equality_DifferentTicket,
	"NEXUS::UnitTests::NCore::FNReportBlock::Equality::DifferentTicket",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNReportBlock A(1);
	const FNReportBlock B(2);
	CHECK_FALSE_MESSAGE(TEXT("Blocks with different tickets should not compare equal."), A == B);
}

N_TEST_HIGH(FNReportBlockTests_GetTicket_ReturnsConstructorValue,
	"NEXUS::UnitTests::NCore::FNReportBlock::GetTicket::ReturnsConstructorValue",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNReportBlock Block(42);
	CHECK_EQUALS("GetTicket returns the value passed to the constructor.", Block.GetTicket(), 42);
}

N_TEST_HIGH(FNReportBlockTests_Heading_Empty_NotRendered,
	"NEXUS::UnitTests::NCore::FNReportBlock::Heading::Empty_NotRendered",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that a block without a heading produces no heading output.
	FNReport Report;
	Report.CreateContentBlock(0, 0);

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::Markdown);
	CHECK_EQUALS("A block with neither heading, header, footer, nor content should emit nothing.", Lines.Num(), 0);
}

N_TEST_HIGH(FNReportBlockTests_Heading_Markdown_Level1,
	"NEXUS::UnitTests::NCore::FNReportBlock::Heading::Markdown_Level1",
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
	Block->SetHeading(TEXT("Title"));

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::Markdown);
	CHECK_EQUALS("Markdown heading should be wrapped by a leading and trailing blank line.",
		Lines.Num(), 3);
	if (Lines.Num() < 3) return;
	CHECK_EQUALS("Leading blank line should precede the markdown heading.",
		Lines[0], FString(TEXT("")));
	CHECK_EQUALS("Markdown level 1 heading should be prefixed with a single '#'.",
		Lines[1], FString(TEXT("# Title")));
	CHECK_EQUALS("Trailing blank line should follow the markdown heading.",
		Lines[2], FString(TEXT("")));
}

N_TEST_HIGH(FNReportBlockTests_Heading_Markdown_Level5,
	"NEXUS::UnitTests::NCore::FNReportBlock::Heading::Markdown_Level5",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that nesting depth selects the matching number of leading '#' in markdown.
	FNReport Report;
	const int32 L1 = Report.CreateContentBlock(0, 0);
	const int32 L2 = Report.CreateContentBlock(L1, 0);
	const int32 L3 = Report.CreateContentBlock(L2, 0);
	const int32 L4 = Report.CreateContentBlock(L3, 0);
	const int32 L5 = Report.CreateContentBlock(L4, 0);
	FNReportContentBlock* Deep = Report.GetContentBlock(L5);
	if (Deep == nullptr)
	{
		ADD_ERROR("Deeply nested content block was not found in the report.");
		return;
	}
	Deep->SetHeading(TEXT("Deep"));

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::Markdown);
	CHECK_MESSAGE(TEXT("Markdown level 5 heading should appear with five '#' characters."),
		Lines.Contains(FString(TEXT("##### Deep"))));
}

N_TEST_HIGH(FNReportBlockTests_Heading_PlainText_Level1,
	"NEXUS::UnitTests::NCore::FNReportBlock::Heading::PlainText_Level1",
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
	Block->SetHeading(TEXT("Hi"));

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::PlainText);
	CHECK_EQUALS("Plain text level 1 heading should emit a leading blank, the heading, and an underline.",
		Lines.Num(), 3);
	if (Lines.Num() < 3) return;
	CHECK_EQUALS("Heading text should appear on the second line.", Lines[1], FString(TEXT("Hi")));
	CHECK_EQUALS("Underline should be one '*' per heading character at level 1.",
		Lines[2], FString(TEXT("**")));
}

N_TEST_MEDIUM(FNReportBlockTests_Heading_PlainText_Level4,
	"NEXUS::UnitTests::NCore::FNReportBlock::Heading::PlainText_Level4",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the inline decoration ('***Heading***') used by plain text level 4.
	FNReport Report;
	const int32 L1 = Report.CreateContentBlock(0, 0);
	const int32 L2 = Report.CreateContentBlock(L1, 0);
	const int32 L3 = Report.CreateContentBlock(L2, 0);
	const int32 L4 = Report.CreateContentBlock(L3, 0);
	FNReportContentBlock* Deep = Report.GetContentBlock(L4);
	if (Deep == nullptr)
	{
		ADD_ERROR("Level 4 content block was not found in the report.");
		return;
	}
	Deep->SetHeading(TEXT("X"));

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::PlainText);
	CHECK_MESSAGE(TEXT("Plain text level 4 heading should be wrapped in '***'."),
		Lines.Contains(FString(TEXT("***X***"))));
}

N_TEST_HIGH(FNReportBlockTests_Header_Markdown,
	"NEXUS::UnitTests::NCore::FNReportBlock::Header::Markdown",
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
	Block->SetHeader(TEXT("Header"));

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::Markdown);
	CHECK_EQUALS("Markdown header should be wrapped by a leading and trailing blank line.",
		Lines.Num(), 3);
	if (Lines.Num() < 3) return;
	CHECK_EQUALS("Leading blank line should precede the markdown header.",
		Lines[0], FString(TEXT("")));
	CHECK_EQUALS("Markdown header should be prefixed with '> '.",
		Lines[1], FString(TEXT("> Header")));
	CHECK_EQUALS("Trailing blank line should follow the markdown header.",
		Lines[2], FString(TEXT("")));
}

N_TEST_HIGH(FNReportBlockTests_Header_PlainText,
	"NEXUS::UnitTests::NCore::FNReportBlock::Header::PlainText",
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
	Block->SetHeader(TEXT("Header"));

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::PlainText);
	CHECK_EQUALS("Plain text header should emit a single line.", Lines.Num(), 1);
	if (Lines.Num() < 1) return;
	CHECK_EQUALS("Plain text header should be emitted verbatim.",
		Lines[0], FString(TEXT("Header")));
}

N_TEST_HIGH(FNReportBlockTests_Footer_Markdown,
	"NEXUS::UnitTests::NCore::FNReportBlock::Footer::Markdown",
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
	Block->SetFooter(TEXT("Footer"));

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::Markdown);
	CHECK_EQUALS("Markdown footer should be wrapped by a leading and trailing blank line.",
		Lines.Num(), 3);
	if (Lines.Num() < 3) return;
	CHECK_EQUALS("Leading blank line should precede the markdown footer.",
		Lines[0], FString(TEXT("")));
	CHECK_EQUALS("Markdown footer should be prefixed with '> '.",
		Lines[1], FString(TEXT("> Footer")));
	CHECK_EQUALS("Trailing blank line should follow the markdown footer.",
		Lines[2], FString(TEXT("")));
}

N_TEST_HIGH(FNReportBlockTests_Footer_PlainText,
	"NEXUS::UnitTests::NCore::FNReportBlock::Footer::PlainText",
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
	Block->SetFooter(TEXT("Footer"));

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::PlainText);
	CHECK_EQUALS("Plain text footer should emit a single line.", Lines.Num(), 1);
	if (Lines.Num() < 1) return;
	CHECK_EQUALS("Plain text footer should be emitted verbatim.",
		Lines[0], FString(TEXT("Footer")));
}

N_TEST_HIGH(FNReportBlockTests_Level_NestedBlock,
	"NEXUS::UnitTests::NCore::FNReportBlock::Level::NestedBlock",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that creating a block as a child of another increments the nesting level by one.
	FNReport Report;
	const int32 Parent = Report.CreateContentBlock(0, 0);
	const int32 Child = Report.CreateContentBlock(Parent, 0);

	FNReportContentBlock* ParentBlock = Report.GetContentBlock(Parent);
	FNReportContentBlock* ChildBlock = Report.GetContentBlock(Child);
	if (ParentBlock == nullptr || ChildBlock == nullptr)
	{
		ADD_ERROR("Could not find newly created blocks in the report.");
		return;
	}
	CHECK_EQUALS("Top-level block should be at level 1.", ParentBlock->GetLevel(), 1);
	CHECK_EQUALS("Block parented to a level 1 block should be at level 2.", ChildBlock->GetLevel(), 2);
}

N_TEST_HIGH(FNReportBlockTests_GetPriority_ReturnsCreationPriority,
	"NEXUS::UnitTests::NCore::FNReportBlock::GetPriority::ReturnsCreationPriority",
	N_TEST_CONTEXT_ANYWHERE)
{
	FNReport Report;
	const int32 Ticket = Report.CreateContentBlock(0, 17);
	FNReportContentBlock* Block = Report.GetContentBlock(Ticket);
	if (Block == nullptr)
	{
		ADD_ERROR("Newly created content block was not found in the report.");
		return;
	}
	CHECK_EQUALS("GetPriority should return the value passed to CreateContentBlock.",
		Block->GetPriority(), 17);
}

#endif //WITH_TESTS

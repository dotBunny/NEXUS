// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Developer/NReport.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNReportContentBlockTests_AddLine_Literal,
	"NEXUS::UnitTests::NCore::FNReportContentBlock::AddLine::Literal",
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
	Block->AddLine(TEXT("Hello"));
	Block->AddLine(TEXT("World"));

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::PlainText);
	// Each content line is followed by a blank separator line.
	CHECK_EQUALS("Two literal lines should produce four output entries (line, blank, line, blank).",
		Lines.Num(), 4);
	if (Lines.Num() < 4) return;
	CHECK_EQUALS("First content line should be the first AddLine value.",
		Lines[0], FString(TEXT("Hello")));
	CHECK_EQUALS("Each content line is followed by a blank separator.",
		Lines[1], FString(TEXT("")));
	CHECK_EQUALS("Second content line should be the second AddLine value.",
		Lines[2], FString(TEXT("World")));
}

N_TEST_HIGH(FNReportContentBlockTests_AddLine_OrderedFormat,
	"NEXUS::UnitTests::NCore::FNReportContentBlock::AddLine::OrderedFormat",
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
	FStringFormatOrderedArguments Args;
	Args.Add(FStringFormatArg(FString(TEXT("World"))));
	Args.Add(FStringFormatArg(42));
	Block->AddLine(TEXT("Hello {0}, value={1}"), Args);

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::PlainText);
	CHECK_MESSAGE(TEXT("Ordered format AddLine should emit at least one line."), Lines.Num() >= 1);
	if (Lines.Num() < 1) return;
	CHECK_EQUALS("Ordered format substitutions should fill in by index.",
		Lines[0], FString(TEXT("Hello World, value=42")));
}

N_TEST_HIGH(FNReportContentBlockTests_AddLine_NamedFormat,
	"NEXUS::UnitTests::NCore::FNReportContentBlock::AddLine::NamedFormat",
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
	FStringFormatNamedArguments Args;
	Args.Add(TEXT("name"), FStringFormatArg(FString(TEXT("Alice"))));
	Args.Add(TEXT("count"), FStringFormatArg(7));
	Block->AddLine(TEXT("{name}={count}"), Args);

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::PlainText);
	CHECK_MESSAGE(TEXT("Named format AddLine should emit at least one line."), Lines.Num() >= 1);
	if (Lines.Num() < 1) return;
	CHECK_EQUALS("Named format substitutions should fill in by token.",
		Lines[0], FString(TEXT("Alice=7")));
}

N_TEST_HIGH(FNReportContentBlockTests_Render_HeaderBodyFooterOrder,
	"NEXUS::UnitTests::NCore::FNReportContentBlock::Render::HeaderBodyFooterOrder",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that header, body, and footer are emitted in that order around the content lines.
	FNReport Report;
	const int32 Ticket = Report.CreateContentBlock(0, 0);
	FNReportContentBlock* Block = Report.GetContentBlock(Ticket);
	if (Block == nullptr)
	{
		ADD_ERROR("Newly created content block was not found in the report.");
		return;
	}
	Block->SetHeader(TEXT("Header"));
	Block->AddLine(TEXT("Body"));
	Block->SetFooter(TEXT("Footer"));

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::PlainText);
	CHECK_EQUALS("Render should emit header, body, blank separator, and footer.", Lines.Num(), 4);
	if (Lines.Num() < 4) return;
	CHECK_EQUALS("Header should appear first.", Lines[0], FString(TEXT("Header")));
	CHECK_EQUALS("Body should follow the header.", Lines[1], FString(TEXT("Body")));
	CHECK_EQUALS("Each content line is followed by a blank separator.", Lines[2], FString(TEXT("")));
	CHECK_EQUALS("Footer should appear last.", Lines[3], FString(TEXT("Footer")));
}

N_TEST_HIGH(FNReportContentBlockTests_Render_NestedChildContent,
	"NEXUS::UnitTests::NCore::FNReportContentBlock::Render::NestedChildContent",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that a parent's body is emitted before its child block content.
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
	ParentBlock->AddLine(TEXT("ParentBody"));
	ChildBlock->AddLine(TEXT("ChildBody"));

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::PlainText);
	int32 ParentIndex = INDEX_NONE;
	int32 ChildIndex = INDEX_NONE;
	for (int32 i = 0; i < Lines.Num(); ++i)
	{
		if (Lines[i] == TEXT("ParentBody")) ParentIndex = i;
		else if (Lines[i] == TEXT("ChildBody")) ChildIndex = i;
	}
	CHECK_MESSAGE(TEXT("Parent body line should appear in the output."), ParentIndex != INDEX_NONE);
	CHECK_MESSAGE(TEXT("Child body line should appear in the output."), ChildIndex != INDEX_NONE);
	CHECK_MESSAGE(TEXT("Parent body should render before its child block content."),
		ParentIndex < ChildIndex);
}

N_TEST_MEDIUM(FNReportContentBlockTests_Render_Markdown_BlankSeparator,
	"NEXUS::UnitTests::NCore::FNReportContentBlock::Render::Markdown_BlankSeparator",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies markdown content rendering uses the same blank-separator pattern as plain text.
	FNReport Report;
	const int32 Ticket = Report.CreateContentBlock(0, 0);
	FNReportContentBlock* Block = Report.GetContentBlock(Ticket);
	if (Block == nullptr)
	{
		ADD_ERROR("Newly created content block was not found in the report.");
		return;
	}
	Block->AddLine(TEXT("Paragraph"));

	const TArray<FString> Lines = Report.GetReportLines(ENReportOutputFormat::Markdown);
	CHECK_EQUALS("Markdown content should emit one body line plus one blank separator.",
		Lines.Num(), 2);
	if (Lines.Num() < 2) return;
	CHECK_EQUALS("Body line should be emitted verbatim in markdown.",
		Lines[0], FString(TEXT("Paragraph")));
	CHECK_EQUALS("Trailing blank separator should follow the body line.",
		Lines[1], FString(TEXT("")));
}

#endif //WITH_TESTS

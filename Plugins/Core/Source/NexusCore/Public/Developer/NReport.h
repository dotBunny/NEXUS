// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

struct FNReport;

/**
 * Output format selector used by FNReport's emit methods to choose between human-readable
 * plain text and Markdown.
 */
enum class ENTextOutputFormat : uint8
{
	/** Plain text formatted for the output log or a console window. */
	PlainText,
	/** Markdown-formatted text suitable for documentation, gists, or ticket attachments. */
	Markdown
};

struct NEXUSCORE_API FNReportContentBlock
{
	FString Heading;
	
	void AddLine(const FString& Line)
	{
		Content.Add(Line);
	}
	void AddLine(const FString& Format, const FStringFormatOrderedArguments& Arguments)
	{
		Content.Add(FString::Format(*Format, Arguments));
	}
	void AddLine(const FString& Format, const FStringFormatNamedArguments& NamedArguments)
	{
		Content.Add(FString::Format(*Format, NamedArguments));
	}
	
	TArray<FString>& GetContent() { return Content; }
	
	FNReportContentBlock& CreateContentBlock();
	FNReportContentBlock& CreateContentBlock(const FString& ContentHeading, int OrderPriority = 0);
	void RemoveContentBlock(const FNReportContentBlock& ChildContentBlock);
	
	bool operator==(const FNReportContentBlock& Other) const
	{
		return Other.Ticket == Ticket && Other.Level == Level;
	}
	

private:
	int32 Level = 0;
	uint64 Ticket = ContentBlockTickets++;
	int32 Priority = 0;
	TArray<FNReportContentBlock> Children;
	TArray<FString> Content;
	static uint64 ContentBlockTickets;
};

struct NEXUSCORE_API FNReport
{
	FNReportContentBlock ContentRoot;
};
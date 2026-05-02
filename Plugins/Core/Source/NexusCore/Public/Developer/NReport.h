// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once


struct FNReport;
struct FNReportHeadingBlock;
struct FNReportContentBlock;
struct FNReportTableBlock;

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

struct NEXUSCORE_API FNReportBlock
{
	FNReportHeadingBlock& CreateHeadingBlock(const FString& Text, int OrderPriority = 0);
	FNReportContentBlock& CreateContentBlock(int OrderPriority = 0);
	FNReportTableBlock& CreateTableBlock(int OrderPriority = 0);
	bool operator==(const FNReportBlock& Other) const
	{
		return Other.Ticket == Ticket && Other.Level == Level;
	}

protected:
	int32 Level = 0;
	int32 Priority = 0;
	TArray<FNReportBlock> Children;

private:
	uint64 Ticket = BlockTickets++;
	static uint64 BlockTickets;
};
struct NEXUSCORE_API FNReportHeadingBlock : FNReportBlock
{
	void SetText(const FString& Text)
	{
		Heading = Text;
	}
private:
	FString Heading;
};
struct NEXUSCORE_API FNReportContentBlock : FNReportBlock
{
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
private:
	TArray<FString> Content;
};
struct NEXUSCORE_API FNReportTableBlock : FNReportBlock
{
	void Initialize(const int32 Columns)
	{
		ColumnCount = Columns;
		
		Header.Empty();
		Header.Reserve(Columns);
		Table.Empty();
	}
	void AddRow(const TArray<FString>& Row)
	{
		Table.Add(Row);
	}
	void SetHeader(const TArray<FString>& HeaderRow)
	{
		Header = HeaderRow;
	}
private:
	TArray<FString> Header;
	TArray<TArray<FString>> Table;
	int32 ColumnCount = 0;
};

struct NEXUSCORE_API FNReport
{
	FNReportBlock ContentRoot;
};
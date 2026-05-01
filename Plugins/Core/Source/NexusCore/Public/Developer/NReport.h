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
	FString Content;
	
	void AddContentBlock(const FNReportContentBlock& ChildContentBlock);
	void RemoveContentBlock(const FNReportContentBlock& ChildContentBlock);
	FNReportContentBlock& CreateContentBlock();
	FNReportContentBlock& CreateContentBlock(const FString& ContentHeading, int OrderPriority = 0);

	bool operator==(const FNReportContentBlock& Other) const
	{
		// TODO: Actual ID?
		return Heading == Other.Heading && Content == Other.Content && Priority == Other.Priority;
	}

private:
	int Priority = 0;
	TArray<FNReportContentBlock> Children;
};

struct NEXUSCORE_API FNReport
{
	FNReportContentBlock ContentRoot;
};
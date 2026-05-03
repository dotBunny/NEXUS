// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NReportOutputFormat.h"

struct FNReportContentBlock;
struct FNReportTableBlock;

/**
 * Base type for all report blocks. Carries the rendering metadata (heading, header, footer,
 * indentation level, sort priority) shared by every block, and tracks an ordered list of
 * child block tickets owned by the parent FNReport.
 */
struct NEXUSCORE_API FNReportBlock
{
	friend struct FNReport;
	explicit FNReportBlock(const int32 Ticket) : BlockTicket(Ticket) {}
	
	
	virtual ~FNReportBlock() = default;

	
	
	bool operator==(const FNReportBlock& Other) const
	{
		return Other.BlockTicket == BlockTicket;
	}

	/** Order priority used by the parent to sort siblings; lower values render first. */
	int32 GetPriority() const { return Priority; }
	int32 GetLevel() const { return Level; }
	
	
	void SetTicket(const int32 Ticket) { BlockTicket = Ticket; }
	int32 GetTicket() const { return BlockTicket; }
	
	
	/** Set the heading line rendered above this block (decorated by level when emitted as plain text or Markdown). */
	void SetHeading(const FString& BlockHeading)
	{
		Heading = BlockHeading;
		HeadingLength = BlockHeading.Len();
	}
	/** Set the header line emitted directly after the heading and before the block body. */
	void SetHeader(const FString& BlockHeader) { Header = BlockHeader; }
	/** Set the footer line emitted after the block body and any child blocks. */
	void SetFooter(const FString& BlockFooter) { Footer = BlockFooter; }

	
	int32 CreateChildContentBlock(int32 OrderPriority = 0);

	int32 CreateChildTableBlock(int32 OrderPriority = 0);

	/**
	 * Render this block (heading, header, children, footer) into Output.
	 * @param Output Line buffer that this block appends to.
	 * @param OutputFormat Whether to emit plain text or Markdown.
	 */
	virtual void Render(FNReport& Report, TArray<FString>& Output, const ENReportOutputFormat OutputFormat = ENReportOutputFormat::PlainText);


protected:
	/** Nesting depth, incremented when this block is created as a child. Drives heading style selection. */
	int32 Level = 0;
	/** Sort priority among siblings; lower values render first. */
	int32 Priority = 0;

	/** Emit the heading line(s), decorated according to Level and OutputFormat. */
	void RenderHeading(TArray<FString>& Output, const ENReportOutputFormat OutputFormat = ENReportOutputFormat::PlainText) const;
	/** Emit the header line, if one was set. */
	void RenderHeader(TArray<FString>& Output, const ENReportOutputFormat OutputFormat = ENReportOutputFormat::PlainText) const;
	/** Emit the footer line, if one was set. */
	void RenderFooter(TArray<FString>& Output, const ENReportOutputFormat OutputFormat = ENReportOutputFormat::PlainText) const;

private:
	FString Heading;
	int32 HeadingLength = 0;
	FString Header;
	FString Footer;

	int32 BlockTicket = -1;
};

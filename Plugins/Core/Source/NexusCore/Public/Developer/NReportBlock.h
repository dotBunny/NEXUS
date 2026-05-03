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

	/** Construct a block with the ticket issued by the owning FNReport; not intended for direct use. */
	explicit FNReportBlock(const int32 Ticket) : BlockTicket(Ticket) {}

	virtual ~FNReportBlock() = default;

	/** Equality is identity-based: two blocks are equal iff they share the same ticket. */
	bool operator==(const FNReportBlock& Other) const
	{
		return Other.BlockTicket == BlockTicket;
	}

	/** Order priority used by the parent to sort siblings; lower values render first. */
	int32 GetPriority() const { return Priority; }

	/** Nesting depth assigned when this block was created (0 = root, 1 = top-level block, etc.). */
	int32 GetLevel() const { return Level; }

	/** Ticket that uniquely identifies this block within its owning FNReport. */
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


	/**
	 * Render this block (heading, header, children, footer) into Output.
	 * @param Report The owning report, used to resolve and render this block's children.
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
	/** Heading text rendered above the block; empty disables heading output. */
	FString Heading;
	/** Cached length of Heading, used to size the underline decoration in plain-text output. */
	int32 HeadingLength = 0;
	/** Optional header line emitted between the heading and the block body. */
	FString Header;
	/** Optional footer line emitted after the block body and any children. */
	FString Footer;

	/** Ticket issued by the owning FNReport; -1 indicates an unparented sentinel. */
	int32 BlockTicket = -1;
};

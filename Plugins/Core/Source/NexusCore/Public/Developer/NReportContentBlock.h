// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Developer/NReportBlock.h"

/**
 * Report block that holds a sequence of free-form text lines. Lines are emitted verbatim
 * for both plain text and Markdown output, separated by a blank line each.
 */
struct NEXUSCORE_API FNReportContentBlock : FNReportBlock
{
	/** Construct a content block with the ticket issued by the owning FNReport; not intended for direct use. */
	explicit FNReportContentBlock(const int32 Ticket)
		: FNReportBlock(Ticket)
	{
	}

	/** Append a literal line to the block. */
	void AddLine(const FString& Line)
	{
		Content.Add(Line);
		Lengths.Add(Content.Last().Len());
	}
	/** Append a line built from FString::Format with ordered arguments. */
	void AddLine(const FString& Format, const FStringFormatOrderedArguments& Arguments)
	{
		Content.Add(FString::Format(*Format, Arguments));
		Lengths.Add(Content.Last().Len());
	}
	/** Append a line built from FString::Format with named arguments. */
	void AddLine(const FString& Format, const FStringFormatNamedArguments& NamedArguments)
	{
		Content.Add(FString::Format(*Format, NamedArguments));
		Lengths.Add(Content.Last().Len());
	}

	/**
	 * Render this content block (heading, header, lines, child blocks, footer) into Output.
	 * @param Report The owning report, used to resolve and render this block's children.
	 * @param Output Line buffer that this block appends to.
	 * @param OutputFormat Whether to emit plain text or Markdown.
	 */
	virtual void Render(FNReport& Report, TArray<FString>& Output, const ENReportOutputFormat OutputFormat = ENReportOutputFormat::PlainText) override;

private:
	/** Body lines, emitted in insertion order with a blank line between entries. */
	TArray<FString> Content;
	/** Per-line character counts captured at insertion time for downstream sizing. */
	TArray<int32> Lengths;
};

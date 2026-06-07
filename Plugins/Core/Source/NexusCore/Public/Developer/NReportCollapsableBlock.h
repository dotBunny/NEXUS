// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Developer/NReportBlock.h"

/**
 * Report block that acts as a collapsing section: it renders its heading, header, child blocks and
 * footer exactly like a base block, but contributes nothing at all when it has no child blocks.
 *
 * Use it to wrap optional groupings whose heading should only appear once something is nested under
 * them — empty sections vanish from the output rather than leaving a dangling heading.
 */
struct NEXUSCORE_API FNReportCollapsableBlock : FNReportBlock
{
	/** Construct a collapsable block with the ticket issued by the owning FNReport; not intended for direct use. */
	explicit FNReportCollapsableBlock(const int32 Ticket)
		: FNReportBlock(Ticket)
	{
	}

	/**
	 * Render this block only when it has child blocks; otherwise emit nothing.
	 * @param Report The owning report, used to resolve and render this block's children.
	 * @param Output Line buffer that this block appends to.
	 * @param OutputFormat Whether to emit plain text or Markdown.
	 */
	virtual void Render(FNReport& Report, TArray<FString>& Output, const ENReportOutputFormat OutputFormat = ENReportOutputFormat::PlainText) override;
};

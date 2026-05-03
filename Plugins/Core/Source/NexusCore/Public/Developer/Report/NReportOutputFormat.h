// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Output format selector used by FNReport's emit methods to choose between human-readable
 * plain text and Markdown.
 */
enum class ENReportOutputFormat : uint8
{
	/** Plain text formatted for the output log or a console window. */
	PlainText,
	/** Markdown-formatted text suitable for documentation, gists, or ticket attachments. */
	Markdown
};
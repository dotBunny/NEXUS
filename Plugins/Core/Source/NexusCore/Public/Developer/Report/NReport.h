// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Developer/Report/NReportBlock.h"
#include "Developer/Report/NReportContentBlock.h"
#include "Developer/Report/NReportTableBlock.h"

/**
 * Composable, hierarchical report structure that can be emitted as plain text or Markdown.
 * Reports own their child blocks (content and table) by value, keyed by an integer ticket
 * issued from a process-wide counter, and a root block establishes the top-level ordering.
 */
struct NEXUSCORE_API FNReport
{
	friend struct FNReportBlock;
	friend struct FNReportTableBlock;
	friend struct FNReportContentBlock;



	/**
	 * Construct a report with a display title.
	 * @param ReportTitle Title used as the report's identifying header.
	 */
	explicit FNReport(FString ReportTitle)
	{
		Title = ReportTitle;
	}

	int32 CreateContentBlock(const int32 ParentTicket = 0, const int32 OrderPriority = 0);
	int32 CreateTableBlock(const int32 ParentTicket = 0, const int32 OrderPriority = 0);
	
	FNReportContentBlock* GetContentBlock(const int32 Ticket);
	FNReportTableBlock* GetTableBlock(const int32 Ticket);

	/**
	 * Render the entire report, walking blocks in priority order, into a flat array of lines.
	 * @param OutputFormat Whether to emit plain text or Markdown.
	 * @return The rendered report, one entry per line.
	 */
	TArray<FString> GetReportLines(ENReportOutputFormat OutputFormat);

protected:
	/** Look up the priority of a block by its ticket; returns 0 when the ticket is unknown. */
	int32 GetPriority(const int32 Ticket);
	
	int32 GetLevel(const int32 Ticket);
	
	void GetOrderedBlocks(int32 ParentTicket, TArray<int32>& Output,  bool bIncludeChildren = true);
	
	/** Render the block identified by Ticket into Output, dispatching to the correct concrete type. */
	void RenderBlock(const int32 Ticket, TArray<FString>& Output, ENReportOutputFormat OutputFormat = ENReportOutputFormat::PlainText);

private:
	FString Title;
	
	/** Storage for every content block in the report, keyed by block ticket. */
	TMap<int32, FNReportContentBlock> ContentBlocks;
	
	/** Storage for every table block in the report, keyed by block ticket. */
	TMap<int32, FNReportTableBlock> TableBlocks;
	
	TMap<int32, TArray<int32>> ChildrenMap;
	
	int32 BlockTickets = 0;
};

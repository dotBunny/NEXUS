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
	 * Allocate a new content block, attach it to a parent, and issue its ticket.
	 * @param ParentTicket Ticket of the block this one is nested under; 0 attaches to the report root.
	 * @param OrderPriority Sort priority among siblings; lower values render first.
	 * @return The ticket assigned to the new block, used with GetContentBlock and as a ParentTicket for children.
	 */
	int32 CreateContentBlock(const int32 ParentTicket = 0, const int32 OrderPriority = 0);

	/**
	 * Allocate a new table block, attach it to a parent, and issue its ticket.
	 * @param ParentTicket Ticket of the block this one is nested under; 0 attaches to the report root.
	 * @param OrderPriority Sort priority among siblings; lower values render first.
	 * @return The ticket assigned to the new block, used with GetTableBlock and as a ParentTicket for children.
	 */
	int32 CreateTableBlock(const int32 ParentTicket = 0, const int32 OrderPriority = 0);

	/**
	 * Look up a previously created content block by ticket.
	 * @param Ticket Ticket returned from CreateContentBlock.
	 * @return Pointer to the block, or nullptr if no content block with that ticket exists.
	 * @note The pointer is owned by the report and is invalidated when another block is created.
	 */
	FNReportContentBlock* GetContentBlock(const int32 Ticket);

	/**
	 * Look up a previously created table block by ticket.
	 * @param Ticket Ticket returned from CreateTableBlock.
	 * @return Pointer to the block, or nullptr if no table block with that ticket exists.
	 * @note The pointer is owned by the report and is invalidated when another block is created.
	 */
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

	/** Look up the nesting level of a block by its ticket; returns 0 for the report root or unknown tickets. */
	int32 GetLevel(const int32 Ticket);

	/**
	 * Collect the tickets of blocks parented to TargetTicket, in priority order, appending them to Output.
	 * @param TargetTicket Ticket whose children should be enumerated; 0 walks the report's top-level blocks.
	 * @param Output Array that ticket entries are appended to.
	 * @param bIncludeChildren When true, recurses into descendants so the entire subtree is flattened.
	 */
	void GetOrderedBlocks(int32 TargetTicket, TArray<int32>& Output, bool bIncludeChildren = true);

	/** Render the block identified by Ticket into Output, dispatching to the correct concrete type. */
	void RenderBlock(const int32 Ticket, TArray<FString>& Output, ENReportOutputFormat OutputFormat = ENReportOutputFormat::PlainText);

private:

	/** Storage for every content block in the report, keyed by block ticket. */
	TMap<int32, FNReportContentBlock> ContentBlocks;

	/** Storage for every table block in the report, keyed by block ticket. */
	TMap<int32, FNReportTableBlock> TableBlocks;

	/** Parent ticket -> ordered child tickets; the root's children live under key 0. */
	TMap<int32, TArray<int32>> ChildrenMap;

	/** Monotonically increasing counter used to issue unique tickets to newly created blocks. */
	int32 BlockTickets = 0;
};

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Developer/NReportBlock.h"
#include "Developer/NReportContentBlock.h"
#include "Developer/NReportTableBlock.h"
#include "Developer/NReportListBlock.h"

/**
 * Composable, hierarchical report structure that can be emitted as plain text or Markdown.
 * Reports own their child blocks (content and table) by value, keyed by an integer ticket
 * issued from a per-report counter; top-level blocks are parented to the implicit root ticket 0.
 */
struct NEXUSCORE_API FNReport
{
	friend struct FNReportBlock;
	friend struct FNReportTableBlock;
	friend struct FNReportContentBlock;
	friend struct FNReportListBlock;

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

	int32 CreateListBlock(const int32 ParentTicket = 0, const int32 OrderPriority = 0);

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

	FNReportListBlock* GetListBlock(const int32 Ticket);

	
	/**
	 * Render the entire report, walking blocks in priority order, into a flat array of lines.
	 * @param OutputFormat Whether to emit plain text or Markdown.
	 * @return The rendered report, one entry per line.
	 */
	TArray<FString> GetReportLines(ENReportOutputFormat OutputFormat = ENReportOutputFormat::PlainText);

	/**
	 * Render the report and write it to disk, creating any missing intermediate directories.
	 * @param FilePath Destination file path; the parent directory tree is created if it does not exist.
	 * @param OutputFormat Whether to emit plain text or Markdown.
	 * @note Logs an error via LogNexusCore on directory creation or file write failure.
	 */
	void OutputToFile(const FString& FilePath, ENReportOutputFormat OutputFormat = ENReportOutputFormat::PlainText);

	/**
	 * Registers a token/value pair to be substituted when the report is rendered, overwriting any existing value for the token.
	 * @param Token The token to search for during rendering.
	 * @param Value The replacement text substituted for the token.
	 */
	void AddReplaceToken(const FString& Token, const FString& Value)
	{
		if (ReplaceTokens.Contains(Token))
		{
			ReplaceTokens[Token] = Value;
			return;
		}
		if (Token.Len() < ShortestReplaceToken)
		{
			ShortestReplaceToken = Token.Len();
		}
		ReplaceTokens.Add(Token, Value);
	}
	void SetDesiredFileName(const FString& FileName) { DesiredFileName = FileName; }
	FString& GetDesiredFileName() { return DesiredFileName; }
	
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
	/** Discriminates which storage map owns a ticket so dispatch resolves a block with a single targeted lookup. */
	enum class EBlockType : uint8 { Content, Table, List };

	/** Per-ticket dispatch and ordering metadata, kept out of the block storage so lookups touch a single small map. */
	struct FBlockMeta
	{
		EBlockType Type = EBlockType::Content;
		int32 Level = 0;
		int32 Priority = 0;
	};

	/** A child reference carrying its priority inline so sibling insertion sorts without any per-comparison map lookup. */
	struct FChildEntry
	{
		int32 Ticket = 0;
		int32 Priority = 0;
	};

	/** Storage for every content block in the report, keyed by block ticket. */
	TMap<int32, FNReportContentBlock> ContentBlocks;

	/** Storage for every table block in the report, keyed by block ticket. */
	TMap<int32, FNReportTableBlock> TableBlocks;
	
	TMap<int32, FNReportListBlock> ListBlocks;

	/** Ticket -> dispatch/ordering metadata for every block, regardless of which storage map holds the block itself. */
	TMap<int32, FBlockMeta> BlockMeta;

	/** Parent ticket -> ordered child entries; the root's children live under key 0. */
	TMap<int32, TArray<FChildEntry>> ChildrenMap;
	
	int32 ShortestReplaceToken = MAX_int32;
	TMap<FString, FString> ReplaceTokens;

	/** Monotonically increasing counter used to issue unique tickets to newly created blocks. */
	int32 BlockTickets = 0;
	
	FString DesiredFileName;
};

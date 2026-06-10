// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NReport.h"

#include "NCoreMinimal.h"
#include "Developer/NReportListBlock.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#define N_REPORT_CREATE_BLOCK(BlockType, BlockStorage, BlockTypeTag) \
	const int32 BlockTicket = ++BlockTickets; \
	BlockStorage.Add(BlockTicket, BlockType(BlockTicket)); \
	BlockType* Block = BlockStorage.Find(BlockTicket); \
	const FBlockMeta* ParentMeta = BlockMeta.Find(ParentTicket); \
	const int32 ParentLevel = ParentMeta != nullptr ? ParentMeta->Level : 0; \
	Block->Level = ParentLevel + 1; \
	Block->Priority = OrderPriority; \
	BlockMeta.Add(BlockTicket, FBlockMeta{ BlockTypeTag, ParentLevel + 1, OrderPriority }); \
	TArray<FChildEntry>& Children = ChildrenMap.FindOrAdd(ParentTicket); \
	int32 InsertIndex = Children.Num(); \
	for (int32 i = 0; i < Children.Num(); i++) \
	{ \
		if (Children[i].Priority > OrderPriority) \
		{ \
			InsertIndex = i; \
			break; \
		} \
	} \
	Children.Insert(FChildEntry{ BlockTicket, OrderPriority }, InsertIndex); \
	return BlockTicket;

int32 FNReport::CreateContentBlock(const int32 ParentTicket, const int32 OrderPriority)
{
	N_REPORT_CREATE_BLOCK(FNReportContentBlock, ContentBlocks, EBlockType::Content)
}

int32 FNReport::CreateTableBlock(const int32 ParentTicket, const int32 OrderPriority)
{
	N_REPORT_CREATE_BLOCK(FNReportTableBlock, TableBlocks, EBlockType::Table)
}

int32 FNReport::CreateListBlock(const int32 ParentTicket, const int32 OrderPriority)
{
	N_REPORT_CREATE_BLOCK(FNReportListBlock, ListBlocks, EBlockType::List)
}

int32 FNReport::CreateCollapsableBlock(const int32 ParentTicket, const int32 OrderPriority)
{
	N_REPORT_CREATE_BLOCK(FNReportCollapsableBlock, CollapsableBlocks, EBlockType::Collapsable)
}

FNReportContentBlock* FNReport::GetContentBlock(const int32 Ticket)
{
	return ContentBlocks.Find(Ticket);
}

FNReportTableBlock* FNReport::GetTableBlock(const int32 Ticket)
{
	return TableBlocks.Find(Ticket);
}

FNReportListBlock* FNReport::GetListBlock(const int32 Ticket)
{
	return ListBlocks.Find(Ticket);
}

FNReportCollapsableBlock* FNReport::GetCollapsableBlock(const int32 Ticket)
{
	return CollapsableBlocks.Find(Ticket);
}

// Heading lives on the base FNReportBlock (private); FNReport is a friend, so the search reads it directly.
#define N_REPORT_FIND_BLOCK(BlockStorage, SearchHeading) \
	for (auto& Pair : BlockStorage) \
	{ \
		if (Pair.Value.Heading.Equals(SearchHeading, ESearchCase::CaseSensitive)) \
		{ \
			return &Pair.Value; \
		} \
	} \
	return nullptr;

FNReportContentBlock* FNReport::FindContentBlock(const FString& Heading)
{
	N_REPORT_FIND_BLOCK(ContentBlocks, Heading)
}

FNReportTableBlock* FNReport::FindTableBlock(const FString& Heading)
{
	N_REPORT_FIND_BLOCK(TableBlocks, Heading)
}

FNReportListBlock* FNReport::FindListBlock(const FString& Heading)
{
	N_REPORT_FIND_BLOCK(ListBlocks, Heading)
}

FNReportCollapsableBlock* FNReport::FindCollapsableBlock(const FString& Heading)
{
	N_REPORT_FIND_BLOCK(CollapsableBlocks, Heading)
}

// As N_REPORT_FIND_BLOCK, but yields the matching block's ticket (the storage key) instead of a pointer.
#define N_REPORT_FIND_BLOCK_TICKET(BlockStorage, SearchHeading) \
	for (auto& Pair : BlockStorage) \
	{ \
		if (Pair.Value.Heading.Equals(SearchHeading, ESearchCase::CaseSensitive)) \
		{ \
			return Pair.Key; \
		} \
	} \
	return INDEX_NONE;

int32 FNReport::FindContentBlockTicket(const FString& Heading)
{
	N_REPORT_FIND_BLOCK_TICKET(ContentBlocks, Heading)
}

int32 FNReport::FindTableBlockTicket(const FString& Heading)
{
	N_REPORT_FIND_BLOCK_TICKET(TableBlocks, Heading)
}

int32 FNReport::FindListBlockTicket(const FString& Heading)
{
	N_REPORT_FIND_BLOCK_TICKET(ListBlocks, Heading)
}

int32 FNReport::FindCollapsableBlockTicket(const FString& Heading)
{
	N_REPORT_FIND_BLOCK_TICKET(CollapsableBlocks, Heading)
}

TArray<FString> FNReport::GetReportLines(const ENReportOutputFormat OutputFormat)
{
	TArray<int32> ChildrenTickets;
	GetOrderedBlocks(0, ChildrenTickets, false);
	
	TArray<FString> Output;
	for (int32 i = 0; i < ChildrenTickets.Num(); i++)
	{
		RenderBlock(ChildrenTickets[i], Output, OutputFormat);
	}
	
	// Early out since we don't have any tokens
	if (ReplaceTokens.IsEmpty())
	{
		return Output;
	}

	// Flatten the token map once so the per-line loop avoids repeated hash lookups; cache each token's length to skip
	// substitutions that cannot possibly fit in the current line.
	TArray<TPair<const FString*, const FString*>> Replacements;
	Replacements.Reserve(ReplaceTokens.Num());
	for (const TPair<FString, FString>& Pair : ReplaceTokens)
	{
		Replacements.Emplace(&Pair.Key, &Pair.Value);
	}

	for (int32 i = 0; i < Output.Num(); i++)
	{
		FString& Line = Output[i];
		if (Line.Len() < ShortestReplaceToken) continue;
		for (int32 j = 0; j < Replacements.Num(); j++)
		{
			const FString& Token = *Replacements[j].Key;
			if (Line.Len() < Token.Len()) continue;
			Line.ReplaceInline(*Token, **Replacements[j].Value, ESearchCase::CaseSensitive);
		}
	}

	return Output;
}

void FNReport::OutputToFile(const FString& FilePath, const ENReportOutputFormat OutputFormat)
{
	const FString Directory = FPaths::GetPath(FilePath);
	if (!Directory.IsEmpty())
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.DirectoryExists(*Directory) && !PlatformFile.CreateDirectoryTree(*Directory))
		{
			UE_LOG(LogNexusCore, Error, TEXT("Unable to create directory '%s' for report output."), *Directory);
			return;
		}
	}

	const TArray<FString> Lines = GetReportLines(OutputFormat);
	if (!FFileHelper::SaveStringArrayToFile(Lines, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8))
	{
		UE_LOG(LogNexusCore, Error, TEXT("Failed to write report to '%s'."), *FilePath);
		return;
	}

	UE_LOG(LogNexusCore, Log, TEXT("Report written to %s."), *FilePath);
}

int32 FNReport::GetPriority(const int32 Ticket)
{
	const FBlockMeta* Meta = BlockMeta.Find(Ticket);
	return Meta != nullptr ? Meta->Priority : 0;
}

int32 FNReport::GetLevel(const int32 Ticket)
{
	const FBlockMeta* Meta = BlockMeta.Find(Ticket);
	return Meta != nullptr ? Meta->Level : 0;
}

void FNReport::GetOrderedBlocks(const int32 TargetTicket, TArray<int32>& Output, const bool bIncludeChildren)
{
	const TArray<FChildEntry>* Children = ChildrenMap.Find(TargetTicket);
	if (Children == nullptr)
	{
		return;
	}

	for (const FChildEntry& Child : *Children)
	{
		Output.Add(Child.Ticket);
		if (bIncludeChildren)
		{
			GetOrderedBlocks(Child.Ticket, Output, true);
		}
	}
}


void FNReport::RenderBlock(const int32 Ticket, TArray<FString>& Output, const ENReportOutputFormat OutputFormat)
{
	const FBlockMeta* Meta = BlockMeta.Find(Ticket);
	if (Meta == nullptr)
	{
		return;
	}

	switch (Meta->Type)
	{
	case EBlockType::Content:
		ContentBlocks[Ticket].Render(*this, Output, OutputFormat);
		break;
	case EBlockType::Table:
		TableBlocks[Ticket].Render(*this, Output, OutputFormat);
		break;
	case EBlockType::List:
		ListBlocks[Ticket].Render(*this, Output, OutputFormat);
		break;
	case EBlockType::Collapsable:
		CollapsableBlocks[Ticket].Render(*this, Output, OutputFormat);
		break;

	}
}
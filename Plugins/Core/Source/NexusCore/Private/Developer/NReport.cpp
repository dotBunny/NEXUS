// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NReport.h"

#include "NCoreMinimal.h"
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

FNReportContentBlock* FNReport::GetContentBlock(const int32 Ticket)
{
	return ContentBlocks.Find(Ticket);
}

FNReportTableBlock* FNReport::GetTableBlock(const int32 Ticket)
{
	return TableBlocks.Find(Ticket);
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
		return MoveTemp(Output);
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

	return MoveTemp(Output);
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
	}
}
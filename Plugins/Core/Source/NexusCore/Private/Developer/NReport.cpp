// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NReport.h"

#include "NCoreMinimal.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#define N_REPORT_CREATE_BLOCK(BlockType, BlockStorage) \
	const int32 BlockTicket = ++BlockTickets; \
	BlockStorage.Add(BlockTicket, BlockType(BlockTicket)); \
	BlockType* Block = BlockStorage.Find(BlockTicket); \
	const int ParentLevel = GetLevel(ParentTicket); \
	Block->Level = ParentLevel + 1; \
	Block->Priority = OrderPriority; \
	if (!ChildrenMap.Contains(ParentTicket)) \
	{ \
		ChildrenMap.Add(ParentTicket, TArray<int32>()); \
	} \
	TArray<int32>& Children = ChildrenMap[ParentTicket]; \
	int32 InsertIndex = Children.Num(); \
	for (int32 i = 0; i < Children.Num(); i++) \
	{ \
		if (GetPriority(Children[i]) > OrderPriority) \
		{ \
			InsertIndex = i; \
			break; \
		} \
	} \
	Children.Insert(BlockTicket, InsertIndex); \
	return BlockTicket;

int32 FNReport::CreateContentBlock(const int32 ParentTicket, const int32 OrderPriority)
{
	N_REPORT_CREATE_BLOCK(FNReportContentBlock, ContentBlocks)
}

int32 FNReport::CreateTableBlock(const int32 ParentTicket, const int32 OrderPriority)
{
	N_REPORT_CREATE_BLOCK(FNReportTableBlock, TableBlocks)
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
	const FNReportContentBlock* ContentBlock = ContentBlocks.Find(Ticket);
	if (ContentBlock != nullptr)
	{
		return ContentBlock->GetPriority();
	}
		
	const FNReportTableBlock* TableBlock = TableBlocks.Find(Ticket);
	if (TableBlock != nullptr)
	{
		return TableBlock->GetPriority();
	}
		
	return 0;
}

int32 FNReport::GetLevel(const int32 Ticket)
{
	const FNReportContentBlock* ContentBlock = ContentBlocks.Find(Ticket);
	if (ContentBlock != nullptr)
	{
		return ContentBlock->GetLevel();
	}
		
	const FNReportTableBlock* TableBlock = TableBlocks.Find(Ticket);
	if (TableBlock != nullptr)
	{
		return TableBlock->GetLevel();
	}
		
	return 0;
}

void FNReport::GetOrderedBlocks(const int32 TargetTicket, TArray<int32>& Output, const bool bIncludeChildren)
{
	const TArray<int32>* Children = ChildrenMap.Find(TargetTicket);
	if (Children == nullptr)
	{
		return;
	}

	for (const int32 ChildTicket : *Children)
	{
		Output.Add(ChildTicket);
		if (bIncludeChildren)
		{
			GetOrderedBlocks(ChildTicket, Output, true);
		}
	}
}


void FNReport::RenderBlock(const int32 Ticket, TArray<FString>& Output, const ENReportOutputFormat OutputFormat)
{
	FNReportContentBlock* ContentBlock = ContentBlocks.Find(Ticket);
	if (ContentBlock != nullptr)
	{
		return ContentBlock->Render(*this, Output, OutputFormat);
	}
		
	FNReportTableBlock* TableBlock = TableBlocks.Find(Ticket);
	if (TableBlock != nullptr)
	{
		return TableBlock->Render(*this, Output, OutputFormat);
	}
}
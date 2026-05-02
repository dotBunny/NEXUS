// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NReport.h"

#define N_REPORT_CREATE_BLOCK(BlockType) \
	int32 InsertIndex = Children.Num(); \
	for (int32 i = 0; i < Children.Num(); i++) \
	{ \
		if (Children[i].Priority > OrderPriority) \
		{ \
			InsertIndex = i; \
			break; \
		} \
	} \
	Children.Insert(BlockType(), InsertIndex); \
	BlockType& Block = static_cast<BlockType&>(Children[InsertIndex]); \
	Block.Priority = OrderPriority; \
	Block.Level = Level + 1;

FNReportHeadingBlock& FNReportBlock::CreateHeadingBlock(const FString& Text, const int OrderPriority)
{
	N_REPORT_CREATE_BLOCK(FNReportHeadingBlock)
	Block.SetText(Text);
	return Block;
}

FNReportContentBlock& FNReportBlock::CreateContentBlock(const int OrderPriority)
{
	N_REPORT_CREATE_BLOCK(FNReportContentBlock)
	return Block;
}

FNReportTableBlock& FNReportBlock::CreateTableBlock(const int OrderPriority)
{
	N_REPORT_CREATE_BLOCK(FNReportTableBlock)
	return Block;
}

uint64 FNReportBlock::BlockTickets = 0;

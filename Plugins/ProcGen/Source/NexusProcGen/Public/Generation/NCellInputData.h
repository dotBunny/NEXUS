// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Cell/NCell.h"
#include "Cell/NCellJunctionDetails.h"
#include "Cell/NCellRootDetails.h"

struct NEXUSPROCGEN_API FNCellInputData
{
	bool bCanBeStartNode= true;
	bool bCanBeEndNode = true;
	
	int MinimumCount = -1;
	int MaximumCount = -1;
	int Weighting = 1;
	
	int UsedCount = 0;
	
	FNCellRootDetails CellDetails;
	
	TMap<int32, FNCellJunctionDetails> Junctions;
	
	// MAIN-THREAD USE ONLY // ROOT OBJECT NEEDS TO BE UNLOADED LATER
	TObjectPtr<UNCell> Template;
	
	bool HasMinimumCount() const { return MinimumCount > -1; }
	bool HasMaximumCount() const { return MaximumCount > -1; }
	bool IsUnique() const { return MinimumCount == 1 && MaximumCount == 1; }
	
	TArray<int32> GetJunctionKeys(const FIntVector2 SocketSize)
	{
		TArray<int32> Keys;
		for (auto& Junction : Junctions)
		{
			if (Junction.Value.SocketSize == SocketSize)
			{
				Keys.Add(Junction.Key);
			}
		}
		return Keys;
	}
	
	TArray<int32> GetJunctionKeys() const
	{
		TArray<int32> Keys;
		Junctions.GetKeys(Keys);
		return MoveTemp(Keys);
	}
};
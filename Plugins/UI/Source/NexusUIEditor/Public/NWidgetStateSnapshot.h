// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NUIEditorMinimal.h"
#include "NWidgetStateSnapshot.generated.h"

USTRUCT(BlueprintType)
struct FNWidgetStateSnapshot
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<FName> Identifiers;
	UPROPERTY()
	TArray<FNWidgetState> WidgetStates;
	
	int GetCount() const
	{
		return Identifiers.Num();
	}
	
	void Clear()
	{
		Identifiers.Empty();
		WidgetStates.Empty();
	}
	
	void RemoveAtIndex(int32 Index)
	{
		Identifiers.RemoveAt(Index);
		WidgetStates.RemoveAt(Index);
	}
	
	int32 GetIdentifierIndex(const FName Identifier) const
	{
		return Identifiers.IndexOfByKey(Identifier);
	}

	bool DuplicateWidgetState(const FName Identifier, const FNWidgetState WidgetState)
	{
		
		return AddWidgetState(Identifier, WidgetState);
	}
	
	bool AddWidgetState(const FName& Identifier, const FNWidgetState& WidgetState)
	{
		// Check for already registered
		int32 WorkingIndex = GetIdentifierIndex(Identifier);
		if (WorkingIndex == INDEX_NONE)
		{
			// Add our items
			WorkingIndex = Identifiers.Add(Identifier);
			const int32 PayloadIndexCheck = WidgetStates.Add(WidgetState);
		
			// Sanity check
			if (WorkingIndex != PayloadIndexCheck)
			{
				UE_LOG(LogNexusUIEditor, Error, TEXT("Sanity check of the known registered widgets's arrays shows inconsistencies; data will be cleared."))
				Clear();
			}
			return true;
		}
		
		WidgetStates[WorkingIndex] = WidgetState;
		return true;
	}
	
	bool RemoveWidgetState(const FName& Identifier)
	{
		const int32 WorkingIndex = GetIdentifierIndex(Identifier);
		if (WorkingIndex != INDEX_NONE)
		{
			Identifiers.RemoveAt(WorkingIndex);
			WidgetStates.RemoveAt(WorkingIndex);
		
			return true;
		}
		return false;
	}
	
	bool UpdateWidgetState(const FName& Identifier,  const FNWidgetState& WidgetState)
	{
		const int32 WorkingIndex = GetIdentifierIndex(Identifier);
		if (WorkingIndex != INDEX_NONE)
		{
			WidgetStates[WorkingIndex] = WidgetState;
			return true;
		}
		UE_LOG(LogNexusUIEditor, Error, TEXT("Failed to update widget(%s)' state; no registered widget was found."), *Identifier.ToString());
		return false;
	}
	
	bool HasWidgetState(const FName& Identifier) const
	{
		return Identifiers.Contains(Identifier);
	}
	
	FNWidgetState& GetWidgetState(const FName& Identifier)
	{
		return WidgetStates[GetIdentifierIndex(Identifier)];
	}
	
	void DumpToLog()
	{
		UE_LOG(LogNexusUIEditor, Log, TEXT("[FNWidgetStateSnapshot]"));
		for (int32 Index = 0; Index < Identifiers.Num(); ++Index)
		{
			UE_LOG(LogNexusUIEditor, Log, TEXT("Identifier(%s)"), *Identifiers[Index].ToString());
			WidgetStates[Index].DumpToLog();
		}
	}
};
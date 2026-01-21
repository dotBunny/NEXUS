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
	TArray<FString> Templates;	
	UPROPERTY()
	TArray<FNWidgetState> WidgetStates;
	
	int GetCount() const
	{
		return Identifiers.Num();
	}
	
	void Clear()
	{
		Identifiers.Empty();
		Templates.Empty();
		WidgetStates.Empty();
	}
	
	void RemoveAtIndex(int32 Index)
	{
		Identifiers.RemoveAt(Index);
		Templates.RemoveAt(Index);
		WidgetStates.RemoveAt(Index);
	}
	
	int32 GetIdentifierIndex(const FName Identifier) const
	{
		return Identifiers.IndexOfByKey(Identifier);
	}

	bool DuplicateWidgetState(const FName Identifier, const FString Template, const FNWidgetState WidgetState)
	{
		
		return AddWidgetState(Identifier, Template, WidgetState);
	}
	
	bool AddWidgetState(const FName& Identifier, const FString& Template,  const FNWidgetState& WidgetState)
	{
		// Check for already registered
		int32 WorkingIndex = GetIdentifierIndex(Identifier);
		if (WorkingIndex == INDEX_NONE)
		{
			// Add our items
			WorkingIndex = Identifiers.Add(Identifier);
			const int32 PayloadIndexCheck = WidgetStates.Add(WidgetState);
			const int32 TemplateIndexCheck = Templates.Add(Template);
		
			// Sanity check
			if (WorkingIndex != PayloadIndexCheck || WorkingIndex != TemplateIndexCheck)
			{
				UE_LOG(LogNexusUIEditor, Error, TEXT("Sanity check of the known registered widgets's arrays shows inconsistencies; data will be cleared."))
				Clear();
			}
			return true;
		}
		
		UE_LOG(LogNexusUIEditor, Warning, TEXT("A widget is already registered for Identifier(%s); updating cached state only."), *Identifier.ToString());
		WidgetStates[WorkingIndex] = WidgetState;
		return true;
	}
	
	bool RemoveWidgetState(const FName& Identifier)
	{
		const int32 WorkingIndex = GetIdentifierIndex(Identifier);
		if (WorkingIndex != INDEX_NONE)
		{
			Identifiers.RemoveAt(WorkingIndex);
			Templates.RemoveAt(WorkingIndex);
			WidgetStates.RemoveAt(WorkingIndex);
		
			return true;
		}
		UE_LOG(LogNexusUIEditor, Warning, TEXT("Failed to unregister widget(%s); no registered widget was found."), *Identifier.ToString());
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
};
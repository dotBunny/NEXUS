// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NUIEditorMinimal.h"
#include "NWidgetStateSnapshot.generated.h"

/**
 * Parallel-arrays snapshot of per-widget state used by UNEditorUtilityWidgetSubsystem. Maps widget
 * identifiers to FNWidgetState key-value bags via two index-aligned arrays; the unusual shape is
 * chosen so UPROPERTY(config) serialization remains straightforward and stable across sessions.
 */
USTRUCT(BlueprintType)
struct FNWidgetStateSnapshot
{
	GENERATED_BODY()

	/** Widget identifiers; parallel to WidgetStates. */
	UPROPERTY()
	TArray<FName> Identifiers;

	/** Per-widget state bags; parallel to Identifiers. */
	UPROPERTY()
	TArray<FNWidgetState> WidgetStates;

	/** @return The number of widgets tracked in this snapshot. */
	int32 GetCount() const
	{
		return Identifiers.Num();
	}

	/** Remove every entry from the snapshot. */
	void Clear()
	{
		Identifiers.Empty();
		WidgetStates.Empty();
	}

	/**
	 * Remove both parallel entries at Index.
	 * @param Index Position in the parallel arrays; not bounds-checked.
	 */
	void RemoveAtIndex(int32 Index)
	{
		Identifiers.RemoveAt(Index);
		WidgetStates.RemoveAt(Index);
	}

	/**
	 * Find the index at which Identifier is stored.
	 * @param Identifier Widget identifier to look up.
	 * @return The matching array index, or INDEX_NONE when not present.
	 */
	int32 GetIdentifierIndex(const FName Identifier) const
	{
		return Identifiers.IndexOfByKey(Identifier);
	}

	/**
	 * Convenience alias for AddWidgetState kept for readability at call sites that are conceptually cloning state.
	 * @param Identifier Widget identifier to store the state under.
	 * @param WidgetState State bag to store.
	 * @return True on success; mirrors AddWidgetState.
	 */
	bool DuplicateWidgetState(const FName Identifier, const FNWidgetState WidgetState)
	{

		return AddWidgetState(Identifier, WidgetState);
	}

	/**
	 * Store WidgetState under Identifier. Updates in place when Identifier already exists; otherwise appends
	 * to both parallel arrays and sanity-checks that the appended indices match.
	 * @param Identifier Widget identifier to key against.
	 * @param WidgetState State bag to store or replace.
	 * @return Always true; return value is retained for symmetry with Remove/Update.
	 * @note On index-mismatch between the parallel arrays the snapshot is cleared as a corruption safeguard.
	 */
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

	/**
	 * Remove the entry for Identifier if present.
	 * @param Identifier Widget identifier to remove.
	 * @return True if an entry was found and removed, false if Identifier was not known.
	 */
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

	/**
	 * Replace the existing state bag for Identifier; logs a warning and returns false if unknown.
	 * @param Identifier Widget identifier expected to already exist in the snapshot.
	 * @param WidgetState New state bag to store.
	 * @return True when the entry was updated, false if Identifier is unknown.
	 */
	bool UpdateWidgetState(const FName& Identifier,  const FNWidgetState& WidgetState)
	{
		const int32 WorkingIndex = GetIdentifierIndex(Identifier);
		if (WorkingIndex != INDEX_NONE)
		{
			WidgetStates[WorkingIndex] = WidgetState;
			return true;
		}
		UE_LOG(LogNexusUIEditor, Warning, TEXT("Failed to update widget(%s)' state; no registered widget was found."), *Identifier.ToString());
		return false;
	}

	/** @return True when Identifier has a stored state bag. */
	bool HasWidgetState(const FName& Identifier) const
	{
		return Identifiers.Contains(Identifier);
	}

	/**
	 * Access the stored state bag for Identifier.
	 * @param Identifier Widget identifier expected to exist in the snapshot.
	 * @return Mutable reference to the stored FNWidgetState.
	 * @note Not bounds-checked; calling with an unknown identifier will assert.
	 */
	FNWidgetState& GetWidgetState(const FName& Identifier)
	{
		return WidgetStates[GetIdentifierIndex(Identifier)];
	}

	/** Dump the entire snapshot to LogNexusUIEditor for diagnostics. */
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
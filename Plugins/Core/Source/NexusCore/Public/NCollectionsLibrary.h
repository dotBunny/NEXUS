// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Collections/NWeightedIntegerArray.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NCollectionsLibrary.generated.h"


/**
 * Blueprint exposure layer for NEXUS collection types.
 *
 * Wraps the inline native API of types declared under Collections/ so they can be created, mutated,
 * and queried from Blueprint graphs. Each entry here is a thin pass-through that exists only because
 * the underlying method is defined on a struct (where UFUNCTION is not available) and therefore
 * cannot be called from Blueprint directly. See the wrapped type's header for the full semantics.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Collections Library")
class UNCollectionsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	/**
	 * Replace the contents of WeightedIntegerArray with the (value, weight) pairs.
	 *
	 * The target array is emptied first, then each entry from Preset.Values is inserted with its
	 * declared weight. Use this to populate a runtime weighted array from a UDataAsset / settings
	 * field authored as a compact map rather than a fully-expanded weighted array.
	 *
	 * @param WeightedIntegerArray The target array, replaced in place.
	 * @param PresetValues         Array of FInvVector2 representing Value(X) and Weight(Y).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Apply Preset")
	static void WeightedIntegerArrayApplyPreset(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray, TArray<FIntVector2> PresetValues)
	{
		WeightedIntegerArray.Empty();
		for (const auto Pair : PresetValues)
		{
			WeightedIntegerArray.Add(Pair.X, Pair.Y);
		}
	}
	
	/**
	 * Insert Weight copies of Value into the weighted array.
	 * @param WeightedIntegerArray The target array, mutated in place.
	 * @param Value                The integer value to insert.
	 * @param Weight               How many copies of Value to insert (the value's relative likelihood of selection).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Add Value")
	static void WeightedIntegerArrayAddValue(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray, const int32 Value, const int32 Weight)
	{
		WeightedIntegerArray.Add(Value, Weight);
	}

	/**
	 * Clear every entry from the weighted array.
	 * @param WeightedIntegerArray The target array, emptied in place.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Empty")
	static void WeightedIntegerArrayEmpty(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray)
	{
		WeightedIntegerArray.Empty();
	}

	/**
	 * Remove every copy of Value from the weighted array.
	 * @param WeightedIntegerArray The target array, mutated in place.
	 * @param Value                The integer value whose copies should all be removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Remove")
	static void WeightedIntegerArrayRemove(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray, const int32 Value)
	{
		WeightedIntegerArray.Remove(Value);
	}

	/**
	 * Remove up to Limit copies of Value from the weighted array.
	 * @param WeightedIntegerArray The target array, mutated in place.
	 * @param Value                The integer value whose copies should be partially removed.
	 * @param Limit                The maximum number of copies to remove. Defaults to 1.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Remove Some")
	static void WeightedIntegerArrayRemoveSome(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray, const int32 Value, const int32 Limit = 1)
	{
		WeightedIntegerArray.RemoveSome(Value, Limit);
	}

	/**
	 * Pick a value using the deterministic FNRandom stream.
	 * @param WeightedIntegerArray The array to pick from.
	 * @return The picked value.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Next Value")
	static int32 WeightedIntegerArrayNextValue(const FNWeightedIntegerArray& WeightedIntegerArray)
	{
		return WeightedIntegerArray.NextValue();
	}

	/**
	 * Pick a value using the deterministic FNRandom stream and remove every copy of it from the array.
	 * @param WeightedIntegerArray The target array, mutated in place — every copy of the returned value is removed.
	 * @return The picked value, which will no longer appear in the array on subsequent calls.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Next Value And Remove")
	static int32 WeightedIntegerArrayNextValueAndRemove(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray)
	{
		return WeightedIntegerArray.NextValueAndRemove();
	}

	/**
	 * Pick a value using the non-deterministic FNRandom stream.
	 * @param WeightedIntegerArray The array to pick from.
	 * @return The picked value.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Random Value")
	static int32 WeightedIntegerArrayRandomValue(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray)
	{
		return WeightedIntegerArray.RandomValue();
	}

	/**
	 * Pick a value using the non-deterministic FNRandom stream and remove every copy of it from the array.
	 * @param WeightedIntegerArray The target array, mutated in place — every copy of the returned value is removed.
	 * @return The picked value, which will no longer appear in the array on subsequent calls.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Random Value And Remove")
	static int32 WeightedIntegerArrayRandomValueAndRemove(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray)
	{
		return WeightedIntegerArray.RandomValueAndRemove();
	}

	/**
	 * Pick a value using a one-shot FRandomStream constructed from Seed.
	 *
	 * Calling again with the same Seed picks the same entry — useful for reproducible single-pick
	 * scenarios where you do not want to thread a stream through your call sites.
	 *
	 * @param WeightedIntegerArray The array to pick from.
	 * @param Seed                 The seed used to construct the FRandomStream for this single pick.
	 * @return The picked value.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Random One Shot Value")
	static int32 WeightedIntegerArrayRandomOneShotValue(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray, const int32 Seed)
	{
		return WeightedIntegerArray.RandomOneShotValue(Seed);
	}

	/**
	 * Pick a value using a one-shot FRandomStream constructed from Seed and remove every copy of it from the array.
	 * @param WeightedIntegerArray The target array, mutated in place — every copy of the returned value is removed.
	 * @param Seed                 The seed used to construct the FRandomStream for this single pick.
	 * @return The picked value, which will no longer appear in the array on subsequent calls.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Random One Shot Value And Remove")
	static int32 WeightedIntegerArrayRandomOneShotValueAndRemove(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray, const int32 Seed)
	{
		return WeightedIntegerArray.RandomOneShotValueAndRemove(Seed);
	}

	/**
	 * Pick a value using an FRandomStream seeded from Seed and write the stream's mutated seed back to the same parameter.
	 *
	 * Use this when chaining picks with a tracked seed so each call advances the stream deterministically.
	 *
	 * @param WeightedIntegerArray The array to pick from.
	 * @param Seed                 The seed used to construct the stream; updated to the post-pick seed on return.
	 * @return The picked value.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Random Tracked Value")
	static int32 WeightedIntegerArrayRandomTrackedValue(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray, UPARAM(ref) int32& Seed)
	{
		return WeightedIntegerArray.RandomTrackedValue(Seed);
	}

	/**
	 * Pick a value using an FRandomStream seeded from Seed, remove every copy of it, and write the stream's mutated seed back.
	 * @param WeightedIntegerArray The target array, mutated in place — every copy of the returned value is removed.
	 * @param Seed                 The seed used to construct the stream; updated to the post-pick seed on return.
	 * @return The picked value, which will no longer appear in the array on subsequent calls.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Random Tracked Value And Remove")
	static int32 WeightedIntegerArrayRandomTrackedValueAndRemove(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray, UPARAM(ref) int32& Seed)
	{
		return WeightedIntegerArray.RandomTrackedValueAndRemove(Seed);
	}

	/**
	 * Whether the weighted array currently holds at least one entry.
	 * @param WeightedIntegerArray The array to query.
	 * @return True if any entries remain, false if the array is empty.
	 */
	UFUNCTION(BlueprintPure, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Has Data")
	static bool WeightedIntegerArrayHasData(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray)
	{
		return WeightedIntegerArray.HasData();
	}

	/**
	 * Whether the weighted array contains at least one copy of Value.
	 * @param WeightedIntegerArray The array to query.
	 * @param Value                The integer value to test for.
	 * @return True if Value is present in the array, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Has Value")
	static bool WeightedIntegerArrayHasValue(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray, const int32 Value)
	{
		return WeightedIntegerArray.HasValue(Value);
	}

	/**
	 * Return the distinct set of values currently present in the weighted array.
	 *
	 * Each value appears once in the result regardless of how many weighted copies it has in the
	 * underlying storage. The returned array is built fresh on every call.
	 *
	 * @param WeightedIntegerArray The array to query.
	 * @return A new array containing each unique value present in the weighted array.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Get Unique Values")
	static TArray<int32> WeightedIntegerArrayGetUniqueValues(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray)
	{
		return WeightedIntegerArray.GetUniqueValues();
	}

	/**
	 * Total number of weighted entries currently in the array.
	 * @param WeightedIntegerArray The array to query.
	 * @return The size of the underlying storage — equal to the sum of every value's remaining weight.
	 */
	UFUNCTION(BlueprintPure, Category = "NEXUS|Collections|Weighted Integer Array", DisplayName = "Weighted Count")
	static int32 WeightedIntegerArrayWeightedCount(UPARAM(ref) FNWeightedIntegerArray& WeightedIntegerArray)
	{
		return WeightedIntegerArray.WeightedCount();
	}

};

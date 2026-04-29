// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NUIMinimal.h"
#include "NWidgetState.generated.h"

/**
 * Lightweight string/bool/float key-value bag used to snapshot and restore UMG widget state
 * across navigation, layer changes, or save/load. Keys are stored in parallel arrays per type
 * so the struct stays USTRUCT-friendly without requiring a container-of-variants.
 */
USTRUCT(BlueprintType)
struct FNWidgetState
{
	GENERATED_BODY()

	/** @return true if a string value is stored for Key. */
	bool HasString(const FString& Key) const
	{
		return StringKeys.Contains(Key);
	}
	/** @return true if a boolean value is stored for Key. */
	bool HasBoolean(const FString& Key) const
	{
		return BooleanKeys.Contains(Key);
	}
	/** @return true if a float value is stored for Key. */
	bool HasFloat(const FString& Key) const
	{
		return FloatKeys.Contains(Key);
	}

	/** @return the stored string for Key, or an empty string if Key is missing. */
	FString GetString(const FString& Key) const
	{
		const int32 Index = StringKeys.IndexOfByKey(Key);
		return Index != INDEX_NONE ? StringValues[Index] : FString();
	}
	/** Append a new string entry without checking for an existing one; returns the new index. */
	int32 AddString(const FString& Key, const FString& Value)
	{
		StringKeys.Add(Key);
		StringValues.Add(Value);
		return StringValues.Num() - 1;
	}
	/** Update or insert the string entry for Key. */
	void SetString(const FString& Key, const FString& Value)
	{
		if (const int32 Index = StringKeys.IndexOfByKey(Key); 
			Index != INDEX_NONE)
		{
			StringValues[Index] = Value;
		}
		else
		{
			AddString(Key, Value);
		}
	}
	/** Remove the string entry for Key if present. */
	void RemoveString(const FString& Key)
	{
		if (const int32 Index = StringKeys.IndexOfByKey(Key);
			Index != INDEX_NONE)
		{
			StringKeys.RemoveAt(Index);
			StringValues.RemoveAt(Index);
		}
	}
	/** Drop every stored string. */
	void ClearStrings()
	{
		StringKeys.Empty();
		StringValues.Empty();
	}

	/** @return the stored boolean for Key, or bDefaultValue if Key is missing. */
	bool GetBoolean(const FString& Key, const bool bDefaultValue = false) const
	{
		const int32 Index = BooleanKeys.IndexOfByKey(Key);
		return Index != INDEX_NONE ? BooleanValues[Index] : bDefaultValue;
	}
	/** Append a new boolean entry without checking for an existing one; returns the new index. */
	int32 AddBoolean(const FString& Key, const bool bValue)
	{
		BooleanKeys.Add(Key);
		BooleanValues.Add(bValue);
		return BooleanValues.Num() - 1;
	}
	/** Update or insert the boolean entry for Key. */
	void SetBoolean(const FString& Key, const bool bValue)
	{
		if (const int32 Index = BooleanKeys.IndexOfByKey(Key); 
			Index != INDEX_NONE)
		{
			BooleanValues[Index] = bValue;
		}
		else
		{
			AddBoolean(Key, bValue);
		}
	}
	/** Remove the boolean entry for Key if present. */
	void RemoveBoolean(const FString& Key)
	{
		if (const int32 Index = BooleanKeys.IndexOfByKey(Key);
			Index != INDEX_NONE)
		{
			BooleanKeys.RemoveAt(Index);
			BooleanValues.RemoveAt(Index);
		}
	}
	/** Drop every stored boolean. */
	void ClearBooleans()
	{
		BooleanKeys.Empty();
		BooleanValues.Empty();
	}

	/** @return the stored float for Key, or DefaultValue if Key is missing. */
	float GetFloat(const FString& Key, const float DefaultValue = 0.f) const
	{
		const int32 Index = FloatKeys.IndexOfByKey(Key);
		return Index != INDEX_NONE ? FloatValues[Index] : DefaultValue;
	}
	/** Append a new float entry without checking for an existing one; returns the new index. */
	int32 AddFloat(const FString& Key, const float Value)
	{
		FloatKeys.Add(Key);
		FloatValues.Add(Value);
		return FloatValues.Num() - 1;
	}
	/** Update or insert the float entry for Key. */
	void SetFloat(const FString& Key, const float Value)
	{
		if (const int32 Index = FloatKeys.IndexOfByKey(Key); 
			Index != INDEX_NONE)
		{
			FloatValues[Index] = Value;
		}
		else
		{
			AddFloat(Key, Value);
		}
	}
	/** Remove the float entry for Key if present. */
	void RemoveFloat(const FString& Key)
	{
		if (const int32 Index = FloatKeys.IndexOfByKey(Key);
			Index != INDEX_NONE)
		{
			FloatKeys.RemoveAt(Index);
			FloatValues.RemoveAt(Index);
		}
	}
	/** Drop every stored float. */
	void ClearFloats()
	{
		FloatKeys.Empty();
		FloatValues.Empty();
	}

	/** Drop every stored string, boolean, and float. */
	void ClearAll()
	{
		ClearBooleans();
		ClearFloats();
		ClearStrings();
	}

	/**
	 * Merge Other into this state.
	 * @param Other Source state whose keys are copied in.
	 * @param bShouldReplaceKeys When true, values from Other overwrite matching keys; otherwise
	 *        existing keys are preserved and only missing keys are copied across.
	 */
	void OverlayState(FNWidgetState& Other, const bool bShouldReplaceKeys = false)
	{
		for (const FString& Key : Other.BooleanKeys)
		{
			if (bShouldReplaceKeys)
			{
				SetBoolean(Key, Other.GetBoolean(Key));
			}
			else
			{
				if (!HasBoolean(Key))
				{
					AddBoolean(Key, Other.GetBoolean(Key));
				}
			}
		}
		
		
		for (const FString& Key : Other.FloatKeys)
		{
			if (bShouldReplaceKeys)
			{
				SetFloat(Key, Other.GetFloat(Key));
			}
			else
			{
				if (!HasFloat(Key))
				{
					AddFloat(Key, Other.GetFloat(Key));
				}
			}
		}
		
		for (const FString& Key : Other.StringKeys)
		{
			if (bShouldReplaceKeys)
			{
				SetString(Key, Other.GetString(Key));
			}
			else
			{
				if (!HasString(Key))
				{
					AddString(Key, Other.GetString(Key));
				}
			}
		}
		
	}

	/** Log every stored key/value pair to LogNexusUI for inspection. */
	void DumpToLog() const
	{
		UE_LOG(LogNexusUI, Log, TEXT("[FNWidgetState]"));
		
		const int32 BooleanCount = BooleanKeys.Num();
		if (BooleanCount > 0)
		{
			UE_LOG(LogNexusUI, Log, TEXT("Booleans(%i)"), BooleanCount);
			for (int32 i = 0; i < BooleanCount; ++i)
			{
				UE_LOG(LogNexusUI, Display, TEXT("  %s : %i"), *BooleanKeys[i], BooleanValues[i]);
			}
		}
		
		const int32 FloatCount = FloatKeys.Num();
		if (FloatCount > 0)
		{
			UE_LOG(LogNexusUI, Log, TEXT("Floats(%i)"), FloatCount);
			for (int32 i = 0; i < FloatCount; ++i)
			{
				UE_LOG(LogNexusUI, Log, TEXT("  %s : %f"), *FloatKeys[i], FloatValues[i]);
			}
		}
		
		const int32 StringCount = StringKeys.Num();
		if (StringCount > 0)
		{
			UE_LOG(LogNexusUI, Log, TEXT("Strings(%i)"), StringCount);
			for (int32 i = 0; i < StringCount; ++i)
			{
				UE_LOG(LogNexusUI, Log, TEXT("  %s : %s"), *StringKeys[i], *StringValues[i]);
			}
		}
	}

protected:

	/** Keys for boolean entries; BooleanKeys[i] maps to BooleanValues[i]. */
	UPROPERTY()
	TArray<FString> BooleanKeys;

	/** Values for boolean entries; index-aligned with BooleanKeys. */
	UPROPERTY()
	TArray<bool> BooleanValues;

	/** Keys for float entries; FloatKeys[i] maps to FloatValues[i]. */
	UPROPERTY()
	TArray<FString> FloatKeys;

	/** Values for float entries; index-aligned with FloatKeys. */
	UPROPERTY()
	TArray<float> FloatValues;

	/** Keys for string entries; StringKeys[i] maps to StringValues[i]. */
	UPROPERTY()
	TArray<FString> StringKeys;

	/** Values for string entries; index-aligned with StringKeys. */
	UPROPERTY()
	TArray<FString> StringValues;
};
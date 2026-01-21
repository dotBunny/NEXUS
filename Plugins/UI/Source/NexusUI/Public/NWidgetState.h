// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NUIMinimal.h"
#include "NWidgetState.generated.h"

USTRUCT(BlueprintType)
struct FNWidgetState
{
	GENERATED_BODY()
	
	bool HasString(const FString& Key) const
	{
		return StringKeys.Contains(Key);
	}
	bool HasBoolean(const FString& Key) const
	{
		return BooleanKeys.Contains(Key);
	}
	bool HasFloat(const FString& Key) const
	{
		return FloatKeys.Contains(Key);
	}
	
	FString GetString(const FString& Key) const
	{
		const int32 Index = StringKeys.IndexOfByKey(Key);
		return Index != INDEX_NONE ? StringValues[Index] : FString();
	}
	int32 AddString(const FString& Key, const FString& Value)
	{
		StringKeys.Add(Key);
		StringValues.Add(Value);
		return StringValues.Num() - 1;
	}
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
	void RemoveString(const FString& Key)
	{
		if (const int32 Index = StringKeys.IndexOfByKey(Key); 
			Index != INDEX_NONE)
		{
			StringKeys.RemoveAt(Index);
			StringValues.RemoveAt(Index);
		}
	}
	void ClearStrings()
	{
		StringKeys.Empty();
		StringValues.Empty();
	}
	
	bool GetBoolean(const FString& Key, const bool bDefaultValue = false) const
	{
		const int32 Index = BooleanKeys.IndexOfByKey(Key);
		return Index != INDEX_NONE ? BooleanValues[Index] : bDefaultValue;
	}
	int32 AddBoolean(const FString& Key, const bool bValue)
	{
		BooleanKeys.Add(Key);
		BooleanValues.Add(bValue);
		return BooleanValues.Num() - 1;
	}
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
	void RemoveBoolean(const FString& Key)
	{
		if (const int32 Index = BooleanKeys.IndexOfByKey(Key); 
			Index != INDEX_NONE)
		{
			BooleanKeys.RemoveAt(Index);
			BooleanValues.RemoveAt(Index);
		}
	}
	void ClearBooleans()
	{
		BooleanKeys.Empty();
		BooleanValues.Empty();
	}
	
	float GetFloat(const FString& Key, const float DefaultValue = 0.f) const
	{
		const int32 Index = FloatKeys.IndexOfByKey(Key);
		return Index != INDEX_NONE ? FloatValues[Index] : DefaultValue;
	}
	int32 AddFloat(const FString& Key, const float Value)
	{
		FloatKeys.Add(Key);
		FloatValues.Add(Value);
		return FloatValues.Num() - 1;
	}
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
	void RemoveFloat(const FString& Key)
	{
		if (const int32 Index = FloatKeys.IndexOfByKey(Key); 
			Index != INDEX_NONE)
		{
			FloatKeys.RemoveAt(Index);
			FloatValues.RemoveAt(Index);
		}
	}
	void ClearFloats()
	{
		FloatKeys.Empty();
		FloatValues.Empty();
	}
	
	void ClearAll()
	{
		ClearBooleans();
		ClearFloats();
		ClearStrings();
	}
	
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

	void DumpToLog() const
	{
		UE_LOG(LogNexusUI, Log, TEXT("[FNWidgetState]"));
		
		const int BooleanCount = BooleanKeys.Num();
		if (BooleanCount > 0)
		{
			UE_LOG(LogNexusUI, Log, TEXT("Booleans(%i)"), BooleanCount);
			for (int i = 0; i < BooleanCount; ++i)
			{
				UE_LOG(LogNexusUI, Display, TEXT("  %s : %i"), *BooleanKeys[i], BooleanValues[i]);
			}
		}
		
		const int FloatCount = FloatKeys.Num();
		if (FloatCount > 0)
		{
			UE_LOG(LogNexusUI, Log, TEXT("Floats(%i)"), FloatCount);
			for (int i = 0; i < FloatCount; ++i)
			{
				UE_LOG(LogNexusUI, Log, TEXT("  %s : %f"), *FloatKeys[i], FloatValues[i]);
			}
		}
		
		const int StringCount = StringKeys.Num();
		if (StringCount > 0)
		{
			UE_LOG(LogNexusUI, Log, TEXT("Strings(%i)"), StringCount);
			for (int i = 0; i < StringCount; ++i)
			{
				UE_LOG(LogNexusUI, Log, TEXT("  %s : %s"), *StringKeys[i], *StringValues[i]);
			}
		}
	}

protected:
	
	UPROPERTY()
	TArray<FString> BooleanKeys;
	
	UPROPERTY()
	TArray<bool> BooleanValues;
	
	UPROPERTY()
	TArray<FString> FloatKeys;
	
	UPROPERTY()
	TArray<float> FloatValues;
	
	UPROPERTY()
	TArray<FString> StringKeys;
	
	UPROPERTY()
	TArray<FString> StringValues;
};
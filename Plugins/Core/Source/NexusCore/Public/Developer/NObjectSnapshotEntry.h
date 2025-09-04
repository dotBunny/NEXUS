// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Misc/EngineVersionComparison.h"
#include "NObjectSnapshotEntry.generated.h"

USTRUCT(BlueprintType)
struct NEXUSCORE_API FNObjectSnapshotEntry
{
	GENERATED_BODY()

	FNObjectSnapshotEntry() = default;
	
	explicit FNObjectSnapshotEntry(const FUObjectItem& Item)
	{
		RefCount = Item.GetRefCount();
		SerialNumber = Item.GetSerialNumber();
#if UE_VERSION_OLDER_THAN(5, 6, 0) // .Object gets deprecated in 5.6
		ObjectPtr = static_cast<UObject*>(Item.Object);
#else
		ObjectPtr = static_cast<UObject*>(Item.GetObject());
#endif		

		bRooted = Item.IsRootSet();
		
		if (ObjectPtr)
		{
			Name = ObjectPtr->GetFName().ToString();
			FullName = ObjectPtr->GetFullName();
		}
		else
		{
			Name = TEXT("N/A");
			FullName = Name;
		}
	}


	bool IsEqual(const FNObjectSnapshotEntry& Other) const
	{
		// Pointer check
		if (ObjectPtr != nullptr && Other.ObjectPtr != nullptr && ObjectPtr == Other.ObjectPtr)
		{
			return true;
		}

		// Check serial next?
		return SerialNumber == Other.SerialNumber;
	}

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	bool bRooted = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TObjectPtr<UObject> ObjectPtr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 SerialNumber = -1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 RefCount = -1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FString Name;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FString FullName;
	
	FString ToString() const
	{
		if (bRooted)
		{
			return "(" + FString::FromInt(RefCount) + ") [R] " + FullName;
		}
		return "(" + FString::FromInt(RefCount) + ") " + FullName;
	}
	FString ToMarkdownTableRow() const
	{
		return "| " + FString::FromInt(RefCount) + " | " + FullName + " |";
	}
};
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
	~FNObjectSnapshotEntry()
	{
		ObjectPtr = nullptr;
	}
	
	explicit FNObjectSnapshotEntry(const FUObjectItem& Item) : bIsRoot(Item.IsRootSet()), bIsGarbage(Item.IsGarbage())
	{
		RefCount = Item.GetRefCount();
		SerialNumber = Item.GetSerialNumber();
#if UE_VERSION_OLDER_THAN(5, 6, 0) // .Object gets deprecated in 5.6
		UObject* Object = static_cast<UObject*>(Item.Object);
#else // UE_VERSION_OLDER_THAN(5, 6, 0)
		UObject* Object = static_cast<UObject*>(Item.GetObject());
#endif // UE_VERSION_OLDER_THAN(5, 6, 0)
		if (Object != nullptr)
		{
			ObjectPtr = Object;
			Name = Object->GetFName().ToString();
			FullName = Object->GetFullName();
		}
		else
		{
			Name = Item.GetObject()->GetFName().ToString();
			FullName = Name;
			ObjectPtr = nullptr;
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
	bool bIsRoot = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	bool bIsGarbage = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TWeakObjectPtr<UObject> ObjectPtr;

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
		return FString::Printf(TEXT("(%i)%hs%hs    %s"), RefCount, bIsRoot ? " [R]" : "    ", bIsGarbage ? " [G]" : "    ", *FullName);
	}
};
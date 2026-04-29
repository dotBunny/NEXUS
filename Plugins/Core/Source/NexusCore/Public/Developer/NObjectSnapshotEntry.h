// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Misc/EngineVersionComparison.h"
#include "NObjectSnapshotEntry.generated.h"

/**
 * A single row within an FNObjectSnapshot, summarizing one UObject at the moment of capture.
 *
 * Stores the minimum identifying information needed to recognize the same object in a later
 * snapshot (pointer, serial number, names) alongside a few flags that are useful for leak
 * diagnosis (root-set membership, garbage flag, ref count). Diff logic (see FNObjectSnapshotUtils)
 * uses IsEqual() to decide whether two entries refer to the same object.
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNObjectSnapshotEntry
{
	GENERATED_BODY()

	FNObjectSnapshotEntry() = default;
	~FNObjectSnapshotEntry()
	{
		ObjectPtr = nullptr;
	}

	/**
	 * Populates an entry by copying identifying data from the supplied FUObjectItem.
	 * @param Item The global object array slot to record.
	 */
	explicit FNObjectSnapshotEntry(const FUObjectItem& Item) : bIsRoot(Item.IsRootSet()), bIsGarbage(Item.IsGarbage())
	{
		RefCount = Item.GetRefCount();
		SerialNumber = Item.GetSerialNumber();
#if UE_VERSION_OLDER_THAN(5, 6, 0) // .Object gets deprecated in 5.6
		UObject* Object = static_cast<UObject*>(Item.Object);
#else // !UE_VERSION_OLDER_THAN(5, 6, 0)
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

	/**
	 * Tests whether two entries describe the same UObject.
	 * @param Other The entry to compare against.
	 * @return true when both entries share a pointer or a serial number.
	 * @note Uses pointer identity first and falls back to serial number when pointers are unavailable.
	 */
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

	/** True when the object was on the engine root set at snapshot time. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	bool bIsRoot = false;

	/** True when the object was flagged for garbage collection at snapshot time. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	bool bIsGarbage = false;

	/** Weak pointer to the captured object; may become invalid after garbage collection. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TWeakObjectPtr<UObject> ObjectPtr;

	/** Serial number of the object's slot, used as an identity fallback when ObjectPtr is stale. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 SerialNumber = -1;

	/** Reference count reported by the global object array at snapshot time. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 RefCount = -1;

	/** Short name (FName as string) of the captured object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FString Name;

	/** Full path name of the captured object, including outer chain. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FString FullName;

	/**
	 * Returns a single-line textual summary suitable for logs and diff output.
	 * @return A string encoding RefCount, Root/Garbage flags, and FullName.
	 */
	FString ToString() const
	{
		return FString::Printf(TEXT("(%i)%hs%hs    %s"), RefCount, bIsRoot ? " [R]" : "    ", bIsGarbage ? " [G]" : "    ", *FullName);
	}
};
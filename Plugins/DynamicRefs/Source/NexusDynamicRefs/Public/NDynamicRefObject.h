// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NDynamicRef.h"
#include "NDynamicRefComponent.h"
#include "NDynamicRefsDeveloperOverlay.h"
#include "NDynamicRefsMinimal.h"
#include "Macros/NValidationMacros.h"
#include "NDynamicRefObject.generated.h"

/**
 * Blueprint-friendly UObject wrapper around a single ENDynamicRef slot or FName bucket.
 *
 * Used primarily by the developer overlay to let UMG bind to a single reference's live object list.
 */
UCLASS(BlueprintType, ClassGroup = "NEXUS", DisplayName = "NEXUS | DynamicRef Object")
class NEXUSDYNAMICREFS_API UNDynamicRefObject : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Factory for an FName-bucket wrapper.
	 * @param Outer The UObject owner (typically the developer overlay).
	 * @param Name The FName bucket this wrapper reflects.
	 */
	static UNDynamicRefObject* Create(UObject* Outer, const FName Name)
	{
		UNDynamicRefObject* Object = NewObject<UNDynamicRefObject>(Outer, StaticClass(), NAME_None, RF_Transient);
		Object->Overlay = Cast<UNDynamicRefsDeveloperOverlay>(Outer);
		Object->TargetName = Name;
		Object->TargetDynamicRef = NDR_None;
		return Object;
	};

	/**
	 * Factory for an ENDynamicRef-slot wrapper.
	 * @param Outer The UObject owner (typically the developer overlay).
	 * @param DynamicRef The ENDynamicRef slot this wrapper reflects.
	 */
	static UNDynamicRefObject* Create(UObject* Outer, const ENDynamicRef DynamicRef)
	{
		UNDynamicRefObject* Object = NewObject<UNDynamicRefObject>(Outer, StaticClass(), NAME_None, RF_Transient);
		Object->Overlay = Cast<UNDynamicRefsDeveloperOverlay>(Outer);
		Object->TargetDynamicRef = DynamicRef;
		Object->TargetName = NAME_None;
		return Object;
	};


	/** @return The ENDynamicRef slot this wrapper targets (meaningful only when TargetName is None). */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|DynamicRefs")
	ENDynamicRef GetDynamicRef() const
	{
		return TargetDynamicRef;
	}

	/** @return The FName bucket this wrapper targets (NAME_None when targeting an ENDynamicRef slot). */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|DynamicRefs")
	FName GetTargetName() const
	{
		return TargetName;
	}

	/**
	 * Append an object to this wrapper's cached list and broadcast Changed.
	 * @param Object The object to add.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|DynamicRefs")
	void AddObject(UObject* Object)
	{
		N_VALIDATE_RETURN_VOID(LogNexusDynamicRefs, Object)
		TargetObjects.AddUnique(Object);
		Changed.ExecuteIfBound();
	}

	/**
	 * Remove an object from this wrapper's cached list and broadcast Changed.
	 * @param Object The object to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|DynamicRefs")
	void RemoveObject(UObject* Object)
	{
		if (TargetObjects.Remove(Object) > 0)
		{
			Changed.ExecuteIfBound();
		}
	}

	/** @return The number of live objects currently tracked by this wrapper (stale entries are pruned first). */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|DynamicRefs")
	int32 GetCount()
	{
		Compact();
		return TargetObjects.Num();
	}

	/** @return A display-friendly label for the target (FName when set, otherwise the ENDynamicRef display name). */
	FText GetReferenceText() const
	{
		if (TargetName != NAME_None)
		{
			return FText::FromName(TargetName);
		}
		return FText::FromString(UNDynamicRefComponent::ToStringSlow(TargetDynamicRef));
	}

	/** @return A mutable view of the tracked objects, with stale entries pruned first. */
	TArray<TWeakObjectPtr<UObject>>& GetObjects()
	{
		Compact();
		return TargetObjects;
	}

	/** Prune entries whose object has been destroyed/GC'd so the wrapper only reflects live objects. */
	void Compact()
	{
		TargetObjects.RemoveAll([](const TWeakObjectPtr<UObject>& Object){ return !Object.IsValid(); });
	}

	/** Fired whenever AddObject/RemoveObject is called so bound UI can refresh. */
	FSimpleDelegate Changed;

	/** @return The overlay this wrapper belongs to, if any. */
	TObjectPtr<UNDynamicRefsDeveloperOverlay> GetOverlay() const { return Overlay; }

private:
	/** The developer overlay that owns this wrapper. */
	UPROPERTY()
	TObjectPtr<UNDynamicRefsDeveloperOverlay> Overlay;

	/** The objects registered for this slot/bucket, mirrored into this wrapper for UI binding. */
	UPROPERTY()
	TArray<TWeakObjectPtr<UObject>> TargetObjects;

	/** The FName bucket this wrapper targets (NAME_None when targeting an ENDynamicRef slot). */
	FName TargetName;

	/** The ENDynamicRef slot this wrapper targets (meaningful only when TargetName is NAME_None). */
	ENDynamicRef TargetDynamicRef;
};
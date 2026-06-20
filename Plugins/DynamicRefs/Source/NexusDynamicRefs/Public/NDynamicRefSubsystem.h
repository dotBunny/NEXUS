// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameplayTagContainer.h"
#include "NDynamicRef.h"
#include "Macros/NSubsystemMacros.h"
#include "NDynamicRefSubsystem.generated.h"

/**
 * Holds the set of UObjects registered for a single ENDynamicRef slot or FName bucket.
 * @remark References are stored weakly; the subsystem never keeps a registered object alive. Entries whose
 *         object is destroyed/GC'd go stale and are skipped by every accessor; they are pruned lazily the next
 *         time an object is added to the same slot/bucket (read accessors never mutate the collection).
 */
USTRUCT()
struct FNDynamicRefCollection
{
	GENERATED_BODY()

	/** The registered UObjects (weak), in registration order. May contain stale entries until the next Add() prunes them. */
	UPROPERTY()
	TArray<TWeakObjectPtr<UObject>> Objects;

	/** Prune entries whose object has been destroyed/GC'd. @return the number of live entries remaining. */
	int32 Compact()
	{
		Objects.RemoveAll([](const TWeakObjectPtr<UObject>& Object){ return !Object.IsValid(); });
		return Objects.Num();
	}
	/** @return true if the collection has at least one live object. */
	bool HasObjects() const
	{
		for (const TWeakObjectPtr<UObject>& Object : Objects)
		{
			if (Object.IsValid()) return true;
		}
		return false;
	}
	/** @return The number of live objects, excluding stale entries, without pruning. */
	int32 CountValid() const
	{
		int32 Count = 0;
		for (const TWeakObjectPtr<UObject>& Object : Objects)
		{
			if (Object.Get()) ++Count;
		}
		return Count;
	}
	/** @return A copy of the currently registered objects as raw pointers, excluding any stale entries. */
	TArray<UObject*> GetObjectsCopy() const
	{
		TArray<UObject*> Result;
		Result.Reserve(Objects.Num());
		for (const TWeakObjectPtr<UObject>& Object : Objects)
		{
			if (UObject* Raw = Object.Get())
			{
				Result.Add(Raw);
			}
		}
		return Result;
	}
	/** @return The first/oldest live object, or nullptr if none remain. */
	UObject* GetFirstValid() const
	{
		for (const TWeakObjectPtr<UObject>& Object : Objects)
		{
			if (UObject* Raw = Object.Get()) return Raw;
		}
		return nullptr;
	}
	/** @return The last/newest live object, or nullptr if none remain. */
	UObject* GetLastValid() const
	{
		for (int32 i = Objects.Num() - 1; i >= 0; --i)
		{
			if (UObject* Raw = Objects[i].Get()) return Raw;
		}
		return nullptr;
	}
	/** Add an object to the collection; duplicates are ignored. @return true if the object was newly added (false when it was already present). */
	bool Add(UObject* Object)
	{
		const int32 PreviousCount = Objects.Num();
		Objects.AddUnique(Object);
		return Objects.Num() > PreviousCount;
	}
	/** Remove an object from the collection if present. */
	bool Remove(UObject* Object) { return Objects.Remove(Object) > 0; }

};

/**
 * A locator system that maintains a map that organizes UObject into predefined categories (ENDynamicRef) or named buckets (FName).
 * @see <a href="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/">UNDynamicRefSubsystem</a>
 */
UCLASS(BlueprintType, ClassGroup = "NEXUS", DisplayName = "NEXUS | DynamicRef Subsystem")
class NEXUSDYNAMICREFS_API UNDynamicRefSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	N_WORLD_SUBSYSTEM_GAME_ONLY(UNDynamicRefSubsystem, true)

	DECLARE_MULTICAST_DELEGATE_TwoParams( FOnDynamicRefChangeDelegate, ENDynamicRef, UObject* );
	DECLARE_MULTICAST_DELEGATE_TwoParams( FOnDynamicRefNameChangeDelegate, FName, UObject* );

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Add a reference by ENDynamicRef to a specified UObject.
	 * @remark Be careful with the manual add method. If you add it, you must remove it!
	 * @param DynamicRef The desired ENDynamicRef to add to.
	 * @param InObject The UObject to be referenced by the provided ENDynamicRef.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Add Object", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#add-object"))
	void AddObject(ENDynamicRef DynamicRef, UObject* InObject);

	/**
	 * Add a reference by ENDynamicRef to a TArray of UObjects.
	 * @remark Be careful with the manual add method. If you add it, you must remove it!
	 * @param DynamicRef The desired ENDynamicRef to add to.
	 * @param InObjects The TArray of UObjects to be referenced by the provided ENDynamicRef.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Add Objects", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#add-objects"))
	void AddObjects(ENDynamicRef DynamicRef, const TArray<UObject*>& InObjects);

	/**
	 * Add a reference by FName to a specified UObject.
	 * @remark Be careful with the manual add method. If you add it, you must remove it!
	 * @param Name The desired FName to add to.
	 * @param InObject The UObject to be referenced by the FName.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Add Object (By Name)", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#add-object-by-name"))
	void AddObjectByName(FName Name, UObject* InObject);

	/**
	 * Add a reference by FName to a TArray of UObjects.
	 * @remark Be careful with the manual add method. If you add it, you must remove it!
	 * @param Name The desired FName to add to.
	 * @param InObjects The TArray of UObjects to be referenced by the FName.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Add Objects (By Name)", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#add-objects-by-name"))
	void AddObjectsByName(FName Name, const TArray<UObject*>& InObjects);

	/**
	 * Remove a reference by ENDynamicRef to a specified UObject.
	 * @param DynamicRef The desired ENDynamicRef to remove from.
	 * @param InObject The UObject to be having its reference removed by the provided ENDynamicRef.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Remove Object", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#remove-object"))
	void RemoveObject(ENDynamicRef DynamicRef, UObject* InObject);

	/**
	 * Remove a reference by ENDynamicRef to a TArray of UObjects.
	 * @param DynamicRef The desired ENDynamicRef to remove from.
	 * @param InObjects TThe TArray of UObjects to be having their references removed by the provided ENDynamicRef.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Remove Objects", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#remove-objects"))
	void RemoveObjects(ENDynamicRef DynamicRef, const TArray<UObject*>& InObjects);

	/**
	 * Remove a reference by FName to a specified UObject.
	 * @remark Be careful with the manual remove method, it should be used for things that you have manually added.
	 * @param Name The desired FName to remove from.
	 * @param InObject The UObject to be having its reference removed by the FName.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Remove Object (By Name)", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#remove-object-by-name"))
	void RemoveObjectByName(FName Name, UObject* InObject);

	/**
	 * Remove a reference by FName to a TArray of UObjects.
	 * @remark Be careful with the manual remove method, it should be used for things that you have manually added.
	 * @param Name The desired FName to remove from.
	 * @param InObjects The TArray of UObjects to be having their references removed by the FName.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Remove Objects (By Name)", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#remove-objects-by-name"))
	void RemoveObjectsByName(FName Name, const TArray<UObject*>& InObjects);

	/**
	 * Gets an array of AActor dynamically associated with the provided ENDynamicRef.
	 * @note This method will only return AActor objects, filtering out any non-AActor UObject.
	 * @param DynamicRef The desired ENDynamicRef to access.
	 * @return An array of UObject.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Actors", Category = "NEXUS|DynamicRefs",
			meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-actors", BlueprintPure="false"))
	TArray<AActor*> GetActors(const ENDynamicRef DynamicRef) const;

	/**
	 * Gets an array of AActor dynamically associated with the provided FName.
	 * @note This method will only return AActor objects, filtering out any non-AActor UObject.
	 * @param Name The desired FName to access.
	 * @return An array of UObject.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Actors (By Name)", Category = "NEXUS|DynamicRefs",
			meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-actors-by-name", BlueprintPure="false"))
	TArray<AActor*> GetActorsByName(FName Name) const;

	/**
	 * Retrieves the count of UObjects associated with a specified ENDynamicRef collection.
	 * @param DynamicRef The desired ENDynamicRef collection.
	 * @return The number of UObjects associated with the specified ENDynamicRef collection.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Count", Category = "NEXUS|DynamicRefs",
			meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-count", BlueprintPure="false"))
	int32 GetCount(const ENDynamicRef DynamicRef) const;

	/**
	 * Retrieves the count of UObjects associated with a specified FName collection.
	 * @param Name The desired FName collection.
	 * @return The number of UObjects associated with the specified FName collection.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Count (By Name)", Category = "NEXUS|DynamicRefs",
			meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-count-by-name", BlueprintPure="false"))
	int32 GetCountByName(FName Name) const;

	/**
	 * Gets an array of UObject dynamically associated with the provided ENDynamicRef.
	 * @param DynamicRef The desired ENDynamicRef to access.
	 * @return An array of UObject.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Objects", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-objects", BlueprintPure="false"))
	TArray<UObject*> GetObjects(const ENDynamicRef DynamicRef) const;

	/**
	 * Gets an array of UObject dynamically associated with the provided FName.
	 * @param Name The desired FName to access.
	 * @return An array of UObject.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Objects (By Name)", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-objects-by-name", BlueprintPure="false"))
	TArray<UObject*> GetObjectsByName(FName Name) const;

	/**
	 * Retrieves the first/oldest AActor associated with a specified ENDynamicRef.
	 * @param DynamicRef The ENDynamicRef collection to iterate.
	 * @return A pointer to the first AActor found for the specified ENDynamicRef, or nullptr if no actors are found.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get First Actor", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-first-actor", BlueprintPure="false"))
	AActor* GetFirstActor(const ENDynamicRef DynamicRef) const;

	/**
	 * Retrieves the first/oldest AActor associated with a specified FName.
	 * @param Name The FName collection to iterate.
	 * @return A pointer to the first AActor found for the specified ENDynamicRef, or nullptr if no actors are found.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get First Actor (By Name)", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-first-actor-by-name", BlueprintPure="false"))
	AActor* GetFirstActorByName(FName Name) const;

	/**
	* Gets the first/oldest UObject associated with the provided ENDynamicRef.
	* @param DynamicRef The desired ENDynamicRef collection to access.
	* @return The first UObject in the collection.
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Get First Object", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-first-object", BlueprintPure="false"))
	UObject* GetFirstObject(const ENDynamicRef DynamicRef) const;

	/**
	* Gets the first/oldest UObject associated with the provided ENDynamicRef with NO bounds/range checking.
	* @note The caller must ensure DynamicRef is valid (not NDR_None/NDR_Max) and the slot is non-empty
	*       (GetCount() > 0). Violating either precondition asserts in development builds and is undefined behavior
	*       in shipping. Use GetFirstObject() for a fully checked lookup.
	* @warning References are weak: if the slot's first entry has been destroyed without removal this returns nullptr even
	*          though the slot is non-empty. Use GetFirstObject() to skip stale entries.
	* @param DynamicRef The desired ENDynamicRef collection to access.
	* @return The first UObject in the collection.
	*/
	UObject* GetFirstObjectUnsafe(const ENDynamicRef DynamicRef) const;

	/**
	* Gets the first/oldest UObject associated with the provided FName.
	* @param Name The desired FName to access.
	* @return The first UObject in the collection.
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Get First Object (By Name)", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-first-object-by-name", BlueprintPure="false"))
	UObject* GetFirstObjectByName(FName Name) const;

	/**
	* Gets the first/oldest UObject associated with the provided FName with NO bounds/range checking.
	* @note The caller must ensure the bucket exists (the FName has been registered) and is non-empty
	*       (GetCountByName() > 0). Violating either precondition asserts in development builds and is undefined
	*       behavior in shipping. Use GetFirstObjectByName() for a fully checked lookup.
	* @warning References are weak: if the bucket's first entry has been destroyed without removal this returns nullptr even
	*          though the bucket is non-empty. Use GetFirstObjectByName() to skip stale entries.
	* @param Name The desired FName to access.
	* @return The first UObject in the collection.
	*/
	UObject* GetFirstObjectByNameUnsafe(FName Name) const;

	/**
	 * Retrieves the last/newest AActor associated with a specified ENDynamicRef.
	 * @param DynamicRef The ENDynamicRef collection to iterate.
	 * @return A pointer to the last AActor found for the specified ENDynamicRef, or nullptr if no actors are found.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Last Actor", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-last-actor", BlueprintPure="false"))
	AActor* GetLastActor(const ENDynamicRef DynamicRef) const;

	/**
	 * Retrieves the last/newest AActor associated with a specified FName.
	 * @param Name The FName collection to iterate.
	 * @return A pointer to the last AActor found for the specified FName, or nullptr if no actors are found.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Last Actor (By Name)", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-last-actor-by-name", BlueprintPure="false"))
	AActor* GetLastActorByName(FName Name) const;

	/**
	* Gets the last/newest UObject associated with the provided ENDynamicRef.
	* @param DynamicRef The desired ENDynamicRef collection to access.
	* @return The last UObject in the collection.
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Get Last Object", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-last-object", BlueprintPure="false"))
	UObject* GetLastObject(const ENDynamicRef DynamicRef) const;

	/**
	* Gets the last/newest UObject associated with the provided ENDynamicRef with NO bounds/range checking.
	* @note The caller must ensure DynamicRef is valid (not NDR_None/NDR_Max) and the slot is non-empty
	*       (GetCount() > 0). Violating either precondition asserts in development builds and is undefined behavior
	*       in shipping. Use GetLastObject() for a fully checked lookup.
	* @warning References are weak: if the slot's last entry has been destroyed without removal this returns nullptr even
	*          though the slot is non-empty. Use GetLastObject() to skip stale entries.
	* @param DynamicRef The desired ENDynamicRef collection to access.
	* @return The last UObject in the collection.
	*/
	UObject* GetLastObjectUnsafe(const ENDynamicRef DynamicRef) const;

	/**
	* Gets the last/newest UObject associated with the provided FName.
	* @param Name The desired FName type to access.
	* @return The last UObject in the collection.
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Get Last Object (By Name)", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-last-object-by-name", BlueprintPure="false"))
	UObject* GetLastObjectByName(FName Name) const;

	/**
	* Gets the last/newest UObject associated with the provided FName with NO bounds/range checking.
	* @note The caller must ensure the bucket exists (the FName has been registered) and is non-empty
	*       (GetCountByName() > 0). Violating either precondition asserts in development builds and is undefined
	*       behavior in shipping. Use GetLastObjectByName() for a fully checked lookup.
	* @warning References are weak: if the bucket's last entry has been destroyed without removal this returns nullptr even
	*          though the bucket is non-empty. Use GetLastObjectByName() to skip stale entries.
	* @param Name The desired FName type to access.
	* @return The last UObject in the collection.
	*/
	UObject* GetLastObjectByNameUnsafe(FName Name) const;


	/** @return All FName buckets that currently have at least one registered object. */
	TArray<FName> GetNames() const;
	/** @return All ENDynamicRef slots that currently have at least one registered object. */
	TArray<ENDynamicRef> GetDynamicRefs() const;
	/**
	 * @return All FGameplayTags whose corresponding FName bucket currently has at least one registered object.
	 * @remark Bucket FNames that do not resolve to a known FGameplayTag (e.g. raw names added via the FName API) are skipped.
	 */
	TArray<FGameplayTag> GetTags() const;

	/**
	 * Gets an array of AActor dynamically associated with the provided FGameplayTag.
	 * @note This method will only return AActor objects, filtering out any non-AActor UObject.
	 * @param Tag The desired FGameplayTag to access.
	 * @return An array of AActor.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Actors (By Tag)", Category = "NEXUS|DynamicRefs", meta=(BlueprintPure="false"))
	TArray<AActor*> GetActorsByTag(FGameplayTag Tag) const;

	/**
	 * Retrieves the count of UObjects associated with a specified FGameplayTag collection.
	 * @param Tag The desired FGameplayTag collection.
	 * @return The number of UObjects associated with the specified FGameplayTag collection.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Count (By Tag)", Category = "NEXUS|DynamicRefs", meta=(BlueprintPure="false"))
	int32 GetCountByTag(FGameplayTag Tag) const;

	/**
	 * Gets an array of UObject dynamically associated with the provided FGameplayTag.
	 * @param Tag The desired FGameplayTag to access.
	 * @return An array of UObject.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Objects (By Tag)", Category = "NEXUS|DynamicRefs", meta=(BlueprintPure="false"))
	TArray<UObject*> GetObjectsByTag(FGameplayTag Tag) const;

	/**
	 * Retrieves the first/oldest AActor associated with a specified FGameplayTag.
	 * @param Tag The FGameplayTag collection to iterate.
	 * @return A pointer to the first AActor found for the specified FGameplayTag, or nullptr if no actors are found.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get First Actor (By Tag)", Category = "NEXUS|DynamicRefs", meta=(BlueprintPure="false"))
	AActor* GetFirstActorByTag(FGameplayTag Tag) const;

	/**
	 * Gets the first/oldest UObject associated with the provided FGameplayTag.
	 * @param Tag The desired FGameplayTag to access.
	 * @return The first UObject in the collection.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get First Object (By Tag)", Category = "NEXUS|DynamicRefs", meta=(BlueprintPure="false"))
	UObject* GetFirstObjectByTag(FGameplayTag Tag) const;

	/**
	 * Gets the first/oldest UObject associated with the provided FGameplayTag with NO bounds/range checking.
	 * @param Tag The desired FGameplayTag to access.
	 * @return The first UObject in the collection.
	 */
	UObject* GetFirstObjectByTagUnsafe(FGameplayTag Tag) const;

	/**
	 * Retrieves the last/newest AActor associated with a specified FGameplayTag.
	 * @param Tag The FGameplayTag collection to iterate.
	 * @return A pointer to the last AActor found for the specified FGameplayTag, or nullptr if no actors are found.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Last Actor (By Tag)", Category = "NEXUS|DynamicRefs", meta=(BlueprintPure="false"))
	AActor* GetLastActorByTag(FGameplayTag Tag) const;

	/**
	 * Gets the last/newest UObject associated with the provided FGameplayTag.
	 * @param Tag The desired FGameplayTag to access.
	 * @return The last UObject in the collection.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Last Object (By Tag)", Category = "NEXUS|DynamicRefs", meta=(BlueprintPure="false"))
	UObject* GetLastObjectByTag(FGameplayTag Tag) const;

	/**
	 * Gets the last/newest UObject associated with the provided FGameplayTag with NO bounds/range checking.
	 * @param Tag The desired FGameplayTag to access.
	 * @return The last UObject in the collection.
	 */
	UObject* GetLastObjectByTagUnsafe(FGameplayTag Tag) const;

	/**
	 * Gets a const reference to the underlying collection for the provided FGameplayTag bucket.
	 * @remark Native code only; prefer this over GetObjectsByTag() when you only need to read the registered objects.
	 * @warning Unsafe: performs no key existence check and will assert/crash (FindChecked) if the tag's bucket does not exist.
	 *          The returned reference is only valid until the next mutation of the named map; any such call may invalidate it.
	 * @param Tag The desired FGameplayTag bucket to access.
	 * @return A const reference to the FNDynamicRefCollection for the bucket.
	 */
	const FNDynamicRefCollection& GetObjectCollectionByTagRefUnsafe(FGameplayTag Tag) const;

	/**
	 * Gets the union of UObjects registered under any of the supplied FGameplayTags. Results are deduplicated.
	 * @param Tags The FGameplayTagContainer whose tags' buckets should be unioned.
	 * @return An array of UObject. Empty if no provided tag has a registered bucket.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Objects (By Any Tags)", Category = "NEXUS|DynamicRefs", meta=(BlueprintPure="false"))
	TArray<UObject*> GetObjectsByAnyTags(const FGameplayTagContainer& Tags) const;

	/**
	 * Gets the union of AActors registered under any of the supplied FGameplayTags. Results are deduplicated and non-AActor UObjects are filtered out.
	 * @param Tags The FGameplayTagContainer whose tags' buckets should be unioned.
	 * @return An array of AActor.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Actors (By Any Tags)", Category = "NEXUS|DynamicRefs", meta=(BlueprintPure="false"))
	TArray<AActor*> GetActorsByAnyTags(const FGameplayTagContainer& Tags) const;

	/**
	 * @param Tags The FGameplayTagContainer whose tags' buckets should be unioned.
	 * @return The number of unique UObjects registered under any of the supplied tags.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Count (By Any Tags)", Category = "NEXUS|DynamicRefs", meta=(BlueprintPure="false"))
	int32 GetCountByAnyTags(const FGameplayTagContainer& Tags) const;

	/**
	 * Gets the intersection of UObjects registered under every supplied FGameplayTag. A UObject must appear under every requested tag to be returned.
	 * @param Tags The FGameplayTagContainer whose tags' buckets should be intersected.
	 * @return An array of UObject. Empty if any provided tag has no registered bucket.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Objects (By All Tags)", Category = "NEXUS|DynamicRefs", meta=(BlueprintPure="false"))
	TArray<UObject*> GetObjectsByAllTags(const FGameplayTagContainer& Tags) const;

	/**
	 * Gets the intersection of AActors registered under every supplied FGameplayTag. Non-AActor UObjects are filtered out.
	 * @param Tags The FGameplayTagContainer whose tags' buckets should be intersected.
	 * @return An array of AActor.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Actors (By All Tags)", Category = "NEXUS|DynamicRefs", meta=(BlueprintPure="false"))
	TArray<AActor*> GetActorsByAllTags(const FGameplayTagContainer& Tags) const;

	/**
	 * @param Tags The FGameplayTagContainer whose tags' buckets should be intersected.
	 * @return The number of UObjects registered under every supplied tag.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Count (By All Tags)", Category = "NEXUS|DynamicRefs", meta=(BlueprintPure="false"))
	int32 GetCountByAllTags(const FGameplayTagContainer& Tags) const;


	/**
	 * Gets a const reference to the underlying collection for the provided ENDynamicRef slot.
	 * @remark Native code only; prefer this over GetObjects() when you only need to read the registered objects, as it avoids copying the array.
	 * @warning Unsafe: performs no bounds checking on DynamicRef and will assert/crash if the value is outside [0, NDR_Max).
	 *          The returned reference is only valid until the next mutation of this slot (Add/Remove); any such call may
	 *          reallocate the backing TArray and invalidate this reference. Do not cache the reference across frames or
	 *          across calls that can modify the subsystem, and do not use it from multiple threads.
	 * @param DynamicRef The desired ENDynamicRef slot to access.
	 * @return A const reference to the FNDynamicRefCollection for the slot.
	 */
	const FNDynamicRefCollection& GetObjectCollectionRefUnsafe(const ENDynamicRef DynamicRef) const;

	/**
	 * Gets a const reference to the underlying collection for the provided FName bucket.
	 * @remark Native code only; prefer this over GetObjectsByName() when you only need to read the registered objects, as it avoids copying the array.
	 * @warning Unsafe: performs no key existence check and will assert/crash (FindChecked) if Name has no registered bucket.
	 *          The returned reference is only valid until the next mutation of the named map (Add/Remove of any bucket);
	 *          a rehash or bucket removal will invalidate this reference. Do not cache the reference across frames or
	 *          across calls that can modify the subsystem, and do not use it from multiple threads.
	 * @param Name The desired FName bucket to access.
	 * @return A const reference to the FNDynamicRefCollection for the bucket.
	 */
	const FNDynamicRefCollection& GetObjectCollectionByNameRefUnsafe(const FName Name) const;

	/**
	 * Fired when an object is added under an ENDynamicRef slot.
	 * @remark Only fired for genuine adds; a duplicate AddUnique is a no-op and does not re-broadcast.
	 */
	FOnDynamicRefChangeDelegate OnAdded;
	/**
	 * Fired when an object is added under an FName bucket.
	 * @remark Only fired for genuine adds; a duplicate AddUnique is a no-op and does not re-broadcast.
	 */
	FOnDynamicRefNameChangeDelegate OnAddedByName;
	/**
	 * Fired when an object is removed from an ENDynamicRef slot.
	 * @remark Only fired for explicit RemoveObject/RemoveObjects calls. Objects that go stale (destroyed/GC'd without
	 *         removal) are pruned lazily on the next Add (see FNDynamicRefCollection) and do NOT broadcast, so the
	 *         add/remove pair is asymmetric for stale departures. Listeners that keep per-object bookkeeping must
	 *         poll/reconcile against the live collection to detect those.
	 */
	FOnDynamicRefChangeDelegate OnRemoved;
	/**
	 * Fired when an object is removed from an FName bucket.
	 * @remark Only fired for explicit RemoveObjectByName/RemoveObjectsByName calls. Objects that go stale (destroyed/GC'd
	 *         without removal) are pruned lazily on the next Add (see FNDynamicRefCollection) and do NOT broadcast, so the
	 *         add/remove pair is asymmetric for stale departures. Listeners that keep per-object bookkeeping must
	 *         poll/reconcile against the live collection to detect those.
	 */
	FOnDynamicRefNameChangeDelegate OnRemovedByName;

private:
	/** Fast array of collections indexed by ENDynamicRef (size == NDR_Max). */
	UPROPERTY()
	TArray<FNDynamicRefCollection> FastCollection;

	/** Named collections, keyed by arbitrary FName. */
	UPROPERTY()
	TMap<FName, FNDynamicRefCollection> NamedCollection;
};
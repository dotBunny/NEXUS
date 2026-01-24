// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDynamicRef.h"
#include "Macros/NSubsystemMacros.h"
#include "NDynamicRefSubsystem.generated.h"

USTRUCT()
struct FNDynamicRefCollection
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<TObjectPtr<UObject>> Objects;
	
	bool HasObjects() const { return Objects.Num() > 0; }
	TArray<UObject*> GetObjects() { return Objects; }
	void Add(UObject* Object) { Objects.AddUnique(Object); }
	void Remove(UObject* Object) { Objects.Remove(Object); }
};

/**
 * A locator system that maintains a map that organizes UObject into predefined categories (ENDynamicRef) or named buckets (FName).
 * @see <a href="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/">UNDynamicRefSubsystem</a>
 */
UCLASS(BlueprintType, ClassGroup = "NEXUS", DisplayName = "DynamicRef Subsystem")
class NEXUSDYNAMICREFS_API UNDynamicRefSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	N_WORLD_SUBSYSTEM_GAME_ONLY(UNDynamicRefSubsystem, true)
	
	/*
	TODO:
	Binding for count change?
	*/
	DECLARE_MULTICAST_DELEGATE_TwoParams( OnDynamicRefCountChangedDelegate, ENDynamicRef, int32 );
	DECLARE_MULTICAST_DELEGATE_TwoParams( OnDynamicRefNameCountChangedDelegate, FName, int32 );
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	/**
	 * Add a reference by ENDynamicRef to a specified UObject.
	 * @remark Be careful with the manual add method. If you add it, you must remove it!
	 * @param DynamicRef The desired ENDynamicRef to add too.
	 * @param InObject The UObject to be referenced by the provided ENDynamicRef.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Add Object", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#add-object"))
	void AddObject(ENDynamicRef DynamicRef, UObject* InObject);

	/**
	 * Add a reference by FName to a specified UObject.
	 * @remark Be careful with the manual add method. If you add it, you must remove it!
	 * @param Name The desired FName to add too.
	 * @param InObject The UObject to be referenced by the FName.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Add Object (By Name)", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#add-object-by-name"))
	void AddObjectByName(FName Name, UObject* InObject);
	
	/**
	 * Remove a reference by ENDynamicRef to a specified UObject.
	 * @param DynamicRef The desired ENDynamicRef to remove from.
	 * @param InObject The UObject to be having its reference removed by the provided ENDynamicRef.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Remove Object", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#remove-object"))
	void RemoveObject(ENDynamicRef DynamicRef, UObject* InObject);
	
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
	 * Gets an array of AActor dynamically associated with the provided ENDynamicRef.
	 * @note This method will only return AActor objects, filtering out any non-AActor UObject.
	 * @param DynamicRef The desired ENDynamicRef to access.
	 * @return An array of UObject. 
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Actors", Category = "NEXUS|DynamicRefs",
			meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-actors"))
	TArray<AActor*> GetActors(const ENDynamicRef DynamicRef);
	
	/**
	 * Gets an array of AActor dynamically associated with the provided FName.
	 * @note This method will only return AActor objects, filtering out any non-AActor UObject.
	 * @param Name The desired FName to access.
	 * @return An array of UObject. 
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Actors (By Name)", Category = "NEXUS|DynamicRefs",
			meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-actors-by-name"))
	TArray<AActor*> GetActorsByName(FName Name);


	/**
	 * Retrieves the count of UObjects associated with a specified ENDynamicRef collection.
	 * @param DynamicRef The desired ENDynamicRef collection.
	 * @return The number of UObjects associated with the specified ENDynamicRef collection.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Count", Category = "NEXUS|DynamicRefs",
			meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-count"))
	int32 GetCount(const ENDynamicRef DynamicRef);

	/**
	 * Retrieves the count of UObjects associated with a specified FName collection.
	 * @param Name The desired FName collection.
	 * @return The number of UObjects associated with the specified FName collection.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Count (By Name)", Category = "NEXUS|DynamicRefs",
			meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-count-by-name"))
	int32 GetCountByName(FName Name);
	
	/**
	 * Gets an array of UObject dynamically associated with the provided ENDynamicRef.
	 * @param DynamicRef The desired ENDynamicRef to access.
	 * @return An array of UObject. 
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Objects", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-objects"))
	TArray<UObject*> GetObjects(const ENDynamicRef DynamicRef);

	/**
	 * Gets an array of UObject dynamically associated with the provided FName.
	 * @param Name The desired FName to access.
	 * @return An array of UObject. 
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Objects", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-objects-by-name"))
	TArray<UObject*> GetObjectsByName(FName Name);

	/**
	 * Retrieves the first/oldest AActor associated with a specified ENDynamicRef.
	 * @param DynamicRef The ENDynamicRef collection to iterate.
	 * @return A pointer to the first AActor found for the specified ENDynamicRef, or nullptr if no actors are found.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get First Actor", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-first-actor"))
	AActor* GetFirstActor(const ENDynamicRef DynamicRef);
	
	/**
	 * Retrieves the first/oldest AActor associated with a specified FName.
	 * @param Name The FName collection to iterate.
	 * @return A pointer to the first AActor found for the specified ENDynamicRef, or nullptr if no actors are found.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get First Actor (By Name)", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-first-actor-by-name"))
	AActor* GetFirstActorByName(FName Name);
	
	/**
	* Gets the first/oldest UObject associated with the provided ENDynamicRef.
	* @param DynamicRef The desired ENDynamicRef collection to access.
	* @return The first UObject in the collection. 
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Get First Object", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-first-object"))
	UObject* GetFirstObject(const ENDynamicRef DynamicRef);
	
	/**
	* Gets the first/oldest UObject associated with the provided ENDynamicRef without any bounds/range checking.
	* @param DynamicRef The desired ENDynamicRef collection to access.
	* @return The first UObject in the collection. 
	*/
	UObject* GetFirstObjectUnsafe(const ENDynamicRef DynamicRef);
	
	/**
	* Gets the first/oldest UObject associated with the provided FName.
	* @param Name The desired FName to access.
	* @return The first UObject in the collection. 
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Get First Object (By Name)", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-first-object-by-name"))
	UObject* GetFirstObjectByName(FName Name);
	
	/**
	* Gets the first/oldest UObject associated with the provided FName without any bounds/range checking.
	* @param Name The desired FName to access.
	* @return The first UObject in the collection. 
	*/
	UObject* GetFirstObjectByNameUnsafe(FName Name);
	
	/**
	 * Retrieves the last/newest AActor associated with a specified ENDynamicRef.
	 * @param DynamicRef The ENDynamicRef collection to iterate.
	 * @return A pointer to the first AActor found for the specified ENDynamicRef, or nullptr if no actors are found.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Last Actor", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-last-actor"))
	AActor* GetLastActor(const ENDynamicRef DynamicRef);
	
	/**
	 * Retrieves the last/newest AActor associated with a specified FName.
	 * @param Name The FName collection to iterate.
	 * @return A pointer to the first AActor found for the specified ENDynamicRef, or nullptr if no actors are found.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Last Actor (By Name)", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-last-actor-by-name"))
	AActor* GetLastActorByName(FName Name);
	
	/**
	* Gets the last/newest UObject associated with the provided ENDynamicRef.
	* @param DynamicRef The desired ENDynamicRef collection to access.
	* @return The last UObject in the collection. 
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Get Last Object", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-last-object"))
	UObject* GetLastObject(const ENDynamicRef DynamicRef);
	
	/**
	* Gets the last/newest UObject associated with the provided ENDynamicRef without any bounds/range checking.
	* @param DynamicRef The desired ENDynamicRef collection to access.
	* @return The first UObject of its type. 
	*/
	UObject* GetLastObjectUnsafe(const ENDynamicRef DynamicRef);
	
	/**
	* Gets the last/newest UObject associated with the provided FName.
	* @param Name The desired FName type to access.
	* @return The last UObject in the collection. 
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Get Last Object (By Name)", Category = "NEXUS|DynamicRefs",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-ref-subsystem/#get-last-object-by-name"))
	UObject* GetLastObjectByName(FName Name);
	
	/**
	* Gets the last/newest UObject associated with the provided FName without any bounds/range checking.
	* @param Name The desired FName type to access.
	* @return The last UObject in the collection. 
	*/
	UObject* GetLastObjectByNameUnsafe(FName Name);

private:
	UPROPERTY()
	TArray<FNDynamicRefCollection> FastCollection;
	
	UPROPERTY()
	TMap<FName, FNDynamicRefCollection> NamedCollection;
};
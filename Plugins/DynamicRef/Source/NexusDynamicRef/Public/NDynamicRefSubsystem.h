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
 * A locator system that maintains a map that organizes UObject's into predefined categories defined by ENDynamicRef or a FName.
 * @see <a href="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-reference-subsystem/">UNDynamicRefSubsystem</a>
 */
UCLASS(BlueprintType, ClassGroup = "NEXUS", DisplayName = "DynamicRef Subsystem")
class NEXUSDYNAMICREF_API UNDynamicRefSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	N_WORLD_SUBSYSTEM_GAME_ONLY(UNDynamicRefSubsystem, true)
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Add a reference by ENDynamicRef to a specified UObject.
	 * @remark Be careful with the manual add method. If you add it, you must remove it!
	 * @param InType The desired NDynamicRef type to add too.
	 * @param InObject The UObject to be referenced by the InType.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Add Fast Reference", Category = "NEXUS|DynamicRef",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-reference-subsystem/#add-fast-reference"))
	void AddFastReference(ENDynamicRef InType, UObject* InObject);

	/**
	 * Remove a reference by ENDynamicRef to a specified UObject.
	 * @param InType The desired NDynamicRef type to remove from.
	 * @param InObject The UObject to be have its reference removed by the InType.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Remove Fast Reference", Category = "NEXUS|DynamicRef",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-reference-subsystem/#remove-fast-reference"))
	void RemoveFastReference(ENDynamicRef InType, UObject* InObject);
	
	/**
	 * Gets the array of UObject dynamically associated with the provided type.
	 * @remark Be careful with the manual remove method, it should be used for things that you have manually added.
	 * @param InType The desired NDynamicRef type to access.
	 * @return An array of UObject. 
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Fast References", Category = "NEXUS|DynamicRef",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-reference-subsystem/#get-fast-references"))
	TArray<UObject*> GetFastReferences(const ENDynamicRef InType);

	/**
	* Gets the first/oldest UObject dynamically associated with the provided type.
	* @param InType The desired NDynamicRef type to access.
	* @return The first UObject of its type. 
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Get Fast Reference (First)", Category = "NEXUS|DynamicRef",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-reference-subsystem/#get-fast-references-first"))
	UObject* GetFastReferenceFirst(const ENDynamicRef InType);
	
	/**
	* Gets the first/oldest UObject dynamically associated with the provided type without any bounds/range checking.
	* @param InType The desired NDynamicRef type to access.
	* @return The first UObject of its type. 
	*/
	UObject* GetFastReferenceFirstUnsafe(const ENDynamicRef InType);
	
	/**
	* Gets the last/newest UObject dynamically associated with the provided type.
	* @param InType The desired NDynamicRef type to access.
	* @return The first UObject of its type. 
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Get Fast Reference (Last)", Category = "NEXUS|DynamicRef",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-reference-subsystem/#get-fast-references-last"))
	UObject* GetFastReferenceLast(const ENDynamicRef InType);
	
	/**
	* Gets the last/newest UObject dynamically associated with the provided type without any bounds/range checking.
	* @param InType The desired NDynamicRef type to access.
	* @return The first UObject of its type. 
	*/
	UObject* GetFastReferenceLastUnsafe(const ENDynamicRef InType);

	
	
	void AddNamedReference(FName Name, UObject* InObject);
	void RemoveNamedReference(FName Name, UObject* InObject);
	
	TArray<UObject*> GetNamedReferences(FName Name);
	UObject* GetNamedReferenceFirst(FName Name);
	UObject* GetNamedReferenceFirstUnsafe(FName Name);
	UObject* GetNamedReferenceLast(FName Name);
	UObject* GetNamedReferenceLastUnsafe(FName Name);
	
	
private:
	UPROPERTY()
	TArray<FNDynamicRefCollection> FastCollection;
	
	UPROPERTY()
	TMap<FName, FNDynamicRefCollection> NamedCollection;
};
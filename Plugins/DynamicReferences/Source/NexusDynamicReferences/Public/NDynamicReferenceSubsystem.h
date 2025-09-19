// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDynamicReference.h"
#include "Macros/NSubsystemMacros.h"
#include "NDynamicReferenceSubsystem.generated.h"


/**
 * A locator system that maintains a map that organizes actors into predefined categories defined by ENDynamicReference.
 * @see <a href="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-reference-subsystem/">UNDynamicReferenceSubsystem</a>
 */
UCLASS(BlueprintType)
class NEXUSDYNAMICREFERENCES_API UNDynamicReferenceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	N_WORLD_SUBSYSTEM_GAME_ONLY(UNDynamicReferenceSubsystem, true)
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Add a reference by ENDynamicReference to a specified AActor.
	 * @remark Be careful with the manual add method. If you add it, you must remove it!
	 * @param InType The desired dynamic reference type to add too.
	 * @param InActor The AActor to be referenced by the InType.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Add Reference", Category = "NEXUS|Dynamic References",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-reference-subsystem/#add-reference"))
	void AddReference(ENDynamicReference InType, AActor* InActor);

	/**
	 * Remove a reference by ENDynamicReference to a specified AActor.
	 * @param InType The desired dynamic reference type to remove from.
	 * @param InActor The AActor to be have its reference removed by the InType.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Remove Reference", Category = "NEXUS|Dynamic References",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-reference-subsystem/#remove-reference"))
	void RemoveReference(ENDynamicReference InType, AActor* InActor);
	
	/**
	 * Gets the array of AActors dynamically associated with the provided type.
	 * @remark Be careful with the manual remove method, it should be used for things that you have manually added.
	 * @param InType The desired dynamic reference type to access.
	 * @return An array of AActors. 
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get References", Category = "NEXUS|Dynamic References",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/dynamic-references/types/dynamic-reference-subsystem/#get-references"))
	TArray<AActor*>& GetReferences(const ENDynamicReference InType) { return ReferenceMap[InType]; }

	
	
private:
	TArray<TArray<AActor*>> ReferenceMap;
};
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NOrganContext.h"
#include "NOrganGenerator.generated.h"

class UNOrganComponent;
class FNOrganGraph;

UCLASS(ClassGroup=(Nexus), DisplayName = "NOrgan Generator")
class NEXUSPROCGEN_API UNOrganGenerator : public UObject
{
	GENERATED_BODY()

	explicit UNOrganGenerator(const FObjectInitializer& ObjectInitializer);
	
public:
	static UNOrganGenerator* CreateInstance(const TArray<TWeakObjectPtr<UObject>>& Objects);
	static UNOrganGenerator* CreateInstance(const TArray<UNOrganComponent*>& Components);
	static UNOrganGenerator* CreateInstance(UNOrganComponent* BaseComponent);
	
	void Reset() const;
	void Generate();
	bool AddToContext(UNOrganComponent* Component) const;
	bool IsLocked() const { return Context->IsLocked(); }
	/**
	 * Lock the context added to the generator and figure out all the generation dependencies and order.
	 */
	void LockContext();
	TArray<TArray<UNOrganComponent*>>& GetGenerationOrder() const
	{
		return Context->GenerationOrder;
	}
	
	FNOrganGraph* GetGraph() const { return Graph; }

protected:
	virtual void BeginDestroy() override;

private:
	FNOrganGraph* Graph;
	FNOrganContext* Context;
	bool bIsContextLocked;
};

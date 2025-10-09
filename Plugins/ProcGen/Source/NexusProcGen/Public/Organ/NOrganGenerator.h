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

	UNOrganGenerator(const FObjectInitializer& ObjectInitializer);
	
public:
	void Reset() const;
	void Generate();
	bool AddToContext(UNOrganComponent* Component) const;

	/**
	 * Lock the context added to the generator and figure out all the generation dependencies and order.
	 */
	void LockContext();
	
	FNOrganGraph* GetGraph() const { return Graph; }

protected:
	virtual void BeginDestroy() override;

private:
	FNOrganGraph* Graph;
	FNOrganContext* Context;
	bool bIsContextLocked;
};

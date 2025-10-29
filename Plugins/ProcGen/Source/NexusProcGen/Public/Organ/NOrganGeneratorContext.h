// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"


class UNOrganComponent;

struct NEXUSPROCGEN_API FNOrganGeneratorContextMap
{
	UNOrganComponent* SourceComponent;
	TArray<UNOrganComponent*> IntersectComponents;
	TArray<UNOrganComponent*> ContainedComponents;
};

/**
 * Game-thread context of the entire generation process.
 */
class NEXUSPROCGEN_API FNOrganGeneratorContext
{
public:
	void Reset();
	bool IsLocked() const { return bIsLocked; }
	void LockAndPreprocess();
	
	bool AddOrganComponent(UNOrganComponent* Component);
	
	void OutputToLog();
	
	TMap<UNOrganComponent*, FNOrganGeneratorContextMap> Components;
	
	TArray<TArray<UNOrganComponent*>> GenerationOrder;
	

	
private:
	bool bIsLocked = false;
};

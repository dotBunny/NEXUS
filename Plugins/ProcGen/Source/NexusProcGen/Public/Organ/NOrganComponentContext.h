// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NOrganComponent.h"

class NEXUSPROCGEN_API FNOrganComponentContext
{
public:
	explicit FNOrganComponentContext(UNOrganComponent* InSourceComponent);
	
	void Reset();
	
	UNOrganComponent* SourceComponent;
	TArray<UNOrganComponent*> OtherComponents;
};

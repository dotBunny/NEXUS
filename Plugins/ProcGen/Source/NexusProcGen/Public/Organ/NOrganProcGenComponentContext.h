// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NProcGenComponent.h"

class NEXUSPROCGEN_API FNOrganProcGenComponentContext
{
public:

	FNOrganProcGenComponentContext(UNProcGenComponent* InSourceComponent);
	
	void Reset();
	
	UNProcGenComponent* SourceComponent;
	TArray<UNProcGenComponent*> OtherComponents;
};

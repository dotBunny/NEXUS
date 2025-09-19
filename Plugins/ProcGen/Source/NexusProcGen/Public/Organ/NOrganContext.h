// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NOrganProcGenComponentContext.h"
#include "NProcGenComponent.h"

class NEXUSPROCGEN_API FNOrganContext
{
public:
	void Reset();

	
	void OutputLockedContext();
	
	TMap<UNProcGenComponent*, FNOrganProcGenComponentContext> Components;
};

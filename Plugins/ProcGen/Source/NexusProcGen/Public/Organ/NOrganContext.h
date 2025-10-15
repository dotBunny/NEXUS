// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NOrganComponentContext.h"

class NEXUSPROCGEN_API FNOrganContext
{
public:
	void Reset();
	bool IsLocked() const { return bIsLocked; }
	void LockAndPreprocess();
	
	bool AddOrganComponent(UNOrganComponent* Component);
	
	void OutputToLog();
	
	TMap<UNOrganComponent*, FNOrganComponentContext> Components;
	
private:
	bool bIsLocked = false;
};

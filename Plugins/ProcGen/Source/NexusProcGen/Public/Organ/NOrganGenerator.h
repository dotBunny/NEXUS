﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NOrganContext.h"
#include "NOrganGenerator.generated.h"

class FNOrganGraph;
class UNProcGenComponent;

UCLASS(ClassGroup=(Nexus), DisplayName = "NOrgan Generator")
class NEXUSPROCGEN_API UNOrganGenerator : public UObject
{
	GENERATED_BODY()

	UNOrganGenerator(const FObjectInitializer& ObjectInitializer);
	
public:
	void Generate();
	bool AddToContext(UNProcGenComponent* Component);
	void LockContext();
	
	FNOrganGraph* GetGraph() const { return Graph; }

protected:
	virtual void BeginDestroy() override;

private:
	FNOrganGraph* Graph;
	FNOrganContext* Context;
	bool bIsContextLocked;
};

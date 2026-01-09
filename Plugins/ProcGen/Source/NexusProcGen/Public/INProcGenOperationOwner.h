// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class UNProcGenOperation;

class INProcGenOperationOwner
{
public:
	INProcGenOperationOwner(){}
	virtual ~INProcGenOperationOwner(){}
	virtual void StartOperation(UNProcGenOperation* Operation) = 0;
	virtual void  OnOperationFinished(UNProcGenOperation* Operation) = 0;
	virtual void  OnOperationDestroyed(UNProcGenOperation* Operation) = 0;
};
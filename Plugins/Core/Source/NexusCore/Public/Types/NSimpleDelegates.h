// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSimpleDelegates.generated.h"

DECLARE_MULTICAST_DELEGATE(FNSimpleMulticastDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNSimpleDynamicMulticastDelegate);

UENUM()
enum class ENSimpleDelegateType : uint8
{
	Multicast = 0,
	DynamicMulticast = 1,
};
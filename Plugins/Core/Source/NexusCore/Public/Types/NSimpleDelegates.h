// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSimpleDelegates.generated.h"

DECLARE_MULTICAST_DELEGATE(FNSimpleMulticastDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNSimpleDynamicMulticastDelegate);

/**
 * Discriminator selecting which of the two parameterless delegate flavors declared above a caller refers to.
 */
UENUM()
enum class ENSimpleDelegateType : uint8
{
	/** Native, non-dynamic multicast delegate (FNSimpleMulticastDelegate). */
	Multicast = 0,
	/** Blueprint-assignable dynamic multicast delegate (FNSimpleDynamicMulticastDelegate). */
	DynamicMulticast = 1,
};
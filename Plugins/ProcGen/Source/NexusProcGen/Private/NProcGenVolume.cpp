// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenVolume.h"
#include "NProcGenComponent.h"
#include "Components/BrushComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NProcGenVolume)

ANProcGenVolume::ANProcGenVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NProcGenComponent = ObjectInitializer.CreateDefaultSubobject<UNProcGenComponent>(this, TEXT("NProcGen Component"));
}

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganVolume.h"
#include "Organ/NOrganComponent.h"
#include "Components/BrushComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NOrganVolume)

ANOrganVolume::ANOrganVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	OrganComponent = ObjectInitializer.CreateDefaultSubobject<UNOrganComponent>(this, TEXT("NOrgan Component"));
}

// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganVolume.h"

#include "Components/BrushComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NOrganVolume)

ANOrganVolume::ANOrganVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	OrganComponent = ObjectInitializer.CreateDefaultSubobject<UNOrganComponent>(this, TEXT("NOrgan Component"));

	BoneComponent = ObjectInitializer.CreateDefaultSubobject<UNBoneComponent>(this, TEXT("NBone Component"));
	BoneComponent->SetupAttachment(this->GetRootComponent());
	BoneComponent->SetMobility(EComponentMobility::Static);
	BoneComponent->SetRelativeLocation(FVector::ZeroVector);
	BoneComponent->OrganComponent = OrganComponent;

#if WITH_EDITORONLY_DATA
	// IMPORTANT: If we  don't always load the organs, the bones will not be found for world assembly.
	bIsSpatiallyLoaded = false;
#endif
}

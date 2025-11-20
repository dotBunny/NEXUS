// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NBoneComponent.h"

void UNBoneComponent::OnTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateTransformFlags,
	ETeleportType Teleport)
{
	FRotator StartRotator = GetComponentRotation();
	StartRotator.Normalize();
	FRotator FinalRotator = FNCardinalDirectionUtils::GetClosestCardinalRotator(StartRotator);
	FinalRotator.Normalize();
	RootRelativeCardinalRotation = FNCardinalRotation::CreateFromNormalized(FinalRotator);
}

void UNBoneComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI) const
{
}

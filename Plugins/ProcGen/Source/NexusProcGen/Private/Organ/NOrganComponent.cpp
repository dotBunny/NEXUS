// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganComponent.h"

#include "NCoreMinimal.h"
#include "NProcGenRegistry.h"
#include "Cell/NCell.h"
#include "Cell/NTissue.h"

FString UNOrganComponent::GetDebugLabel() const
{
	return GetOwner()->GetActorNameOrLabel();
}

FNPositionRotation UNOrganComponent::GetDebugLabelPositionRotation() const
{
	FNPositionRotation PositionRotation;
	const AActor* Owner = GetOwner();
	
	if (IsVolumeBased())
	{
		const AVolume* Volume = Cast<AVolume>(Owner);
		const FBox VolumeBox = Volume->GetBounds().GetBox();
		PositionRotation.Position = FVector(VolumeBox.Min.X, VolumeBox.Min.Y, VolumeBox.Max.Z);
	}
	else
	{
		PositionRotation.Position = Owner->GetActorLocation();
		PositionRotation.Rotation = Owner->GetActorRotation();
	}
	
	return MoveTemp(PositionRotation);
}

void UNOrganComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI) const
{
	if (IsVolumeBased())
	{
		const AActor* Owner = GetOwner();
		FTransform BoxTransform = Owner->GetLevelTransform();
		const auto Box =  Cast<AVolume>(GetOwner())->GetBounds().GetBox();
		BoxTransform.SetLocation( Owner->GetActorLocation() );
		DrawBox( PDI, BoxTransform.ToMatrixWithScale(), Box.GetExtent(), GEngine->ConstraintLimitMaterialPrismatic->GetRenderProxy(), SDPG_World );
	}
}

void UNOrganComponent::OnRegister()
{
	FNProcGenRegistry::RegisterOrganComponent(this);
	Super::OnRegister();
}

void UNOrganComponent::OnUnregister()
{
	FNProcGenRegistry::UnregisterOrganComponent(this);
	Super::OnUnregister();
}

TMap<TObjectPtr<UNCell>, FNTissueEntry> UNOrganComponent::GetTissueMap() const
{
	TMap<TObjectPtr<UNCell>, FNTissueEntry> TissueMap;
	TArray<UNTissue*> ReferencedTissues;
	for (auto Tissue : Tissues)
	{
		UNTissue::BuildTissueMap(Tissue, TissueMap, ReferencedTissues);
	}
	return MoveTemp(TissueMap);
}

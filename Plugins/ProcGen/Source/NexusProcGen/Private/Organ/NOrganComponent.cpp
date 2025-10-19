// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganComponent.h"

#include "NCoreMinimal.h"
#include "NProcGenRegistry.h"
#include "Cell/NCell.h"
#include "Tissue/NTissue.h"

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

void UNOrganComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI, const FLinearColor Color, const float DepthBias) const
{
	// TODO: this needs to get called when you have a organ selected, it should iterate the contained organs and color them specifically with less bias, lighter color?
	if (IsVolumeBased())
	{
		const AVolume* Volume = Cast<AVolume>(GetOwner());
		DrawWireBox(PDI,  Volume->GetBounds().GetBox(), Color, SDPG_World, 2.f, DepthBias);
	}
	else
	{
		N_LOG(Log, TEXT("[UNOrganComponent::DrawDebugPDI] %s is not volume based and is unable to draw."), *this->GetDebugLabel())
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
	for (const auto& Set : ReferencedTissues)
	{
		N_LOG(Log, TEXT("[UNOrganComponent::GetTissueMap] %s References UNTissue: %s"), *this->GetName(), *Set->GetName())
	}
	return MoveTemp(TissueMap);
}

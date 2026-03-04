// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganComponent.h"

#include "DynamicMeshBuilder.h"
#include "NCoreMinimal.h"
#include "NProcGenRegistry.h"
#include "Cell/NCell.h"
#include "Cell/NTissue.h"
#include "Components/BrushComponent.h"
#include "Math/NVolumeUtils.h"

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
		const AVolume* Volume = Cast<AVolume>(Owner);
		const auto Box =  Volume->GetBounds().GetBox();
		BoxTransform.SetLocation( Owner->GetActorLocation() );
		
		UModel* Model = Volume->GetBrushComponent()->Brush;
		
		FNVolumeGeometryData Data;
		FNVolumeUtils::FillGeometryData(Model, Data);
		
		FDynamicMeshBuilder MeshBuilder(PDI->View->GetFeatureLevel());
		MeshBuilder.AddVertices(Data.Vertices);
		for (int32 i = 0; i < Data.Indices.Num(); i += 3)
		{
			MeshBuilder.AddTriangle(
				Data.Indices[i],
				Data.Indices[i+1],
				Data.Indices[i+2]
			);
		}
		MeshBuilder.Draw(PDI, Volume->GetBrushComponent()->GetRenderMatrix(), 
			GEngine->ConstraintLimitMaterialPrismatic->GetRenderProxy(), SDPG_World);
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

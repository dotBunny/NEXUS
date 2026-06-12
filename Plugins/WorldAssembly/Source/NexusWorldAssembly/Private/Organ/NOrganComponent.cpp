// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganComponent.h"

#include "DynamicMeshBuilder.h"
#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblySubsystem.h"
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
	
	const AVolume* Volume = IsVolumeBased() ? Cast<AVolume>(Owner) : nullptr;
	if (Volume != nullptr)
	{
		const FBox VolumeBox = Volume->GetBounds().GetBox();
		PositionRotation.Position = FVector(VolumeBox.Min.X, VolumeBox.Min.Y, VolumeBox.Max.Z);
	}
	else
	{
		PositionRotation.Position = Owner->GetActorLocation();
		PositionRotation.Rotation = Owner->GetActorRotation();
	}
	
	return PositionRotation;
}

void UNOrganComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI) const
{
	if (IsVolumeBased())
	{
		const AActor* Owner = GetOwner();
		const AVolume* Volume = Cast<AVolume>(Owner);
		if (Volume == nullptr) return;

		FTransform BoxTransform = Owner->GetLevelTransform();
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
		if (bUnbound)
		{
			MeshBuilder.Draw(PDI, Volume->GetBrushComponent()->GetRenderMatrix(), 
			GEngine->ConstraintLimitMaterial->GetRenderProxy(), SDPG_World);
		}
		else
		{
			MeshBuilder.Draw(PDI, Volume->GetBrushComponent()->GetRenderMatrix(), 
			GEngine->ConstraintLimitMaterialPrismatic->GetRenderProxy(), SDPG_World);
		}
		
	}
}

void UNOrganComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GenerationTrigger == ENOrganGenerationTrigger::BeginPlay)
	{
		UNWorldAssemblySubsystem::Get(GetWorld())->RegisterOrganForAssembly(this);
	}
}

void UNOrganComponent::OnRegister()
{
	FNWorldAssemblyRegistry::RegisterOrganComponent(this);
	Super::OnRegister();
}

void UNOrganComponent::OnUnregister()
{
	FNWorldAssemblyRegistry::UnregisterOrganComponent(this);
	Super::OnUnregister();
}

#if WITH_EDITOR
void UNOrganComponent::PostEditImport()
{
	Super::PostEditImport();

	// Editor duplication (alt-drag) and copy/paste both go through text import, which carries the serialized
	// Identifier across to the copy. Regenerate it so duplicated organs don't share the GUID used for
	// deterministic ordering and as the OrganResults / OrganCellCount map key. This hook does not fire on load,
	// so the source organ keeps its identifier across saves.
	Identifier = FGuid::NewGuid();
}
#endif // WITH_EDITOR

void UNOrganComponent::GetTissueMap(TMap<TObjectPtr<UNCell>, FNTissueEntry>& OutMap, FNTissueTagGroups& OutTagGroups) const
{
	TArray<UNTissue*> ReferencedTissues;
	for (auto Tissue : Tissues)
	{
		TObjectPtr<UNTissue> LoadedTissue = Tissue.LoadSynchronous();
		if (LoadedTissue == nullptr) continue;
		UNTissue::BuildTissueMap(LoadedTissue, OutMap, OutTagGroups, ReferencedTissues);
	}
}
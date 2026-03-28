// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Elements/NGetJunctionDataElement.h"

#include "NProcGenRegistry.h"
#include "NProcGenSettings.h"
#include "PCGContext.h"
#include "Cell/NCellJunctionComponent.h"
#include "Cell/NCellJunctionDetails.h"
#include "Data/PCGPointData.h"

TArray<FPCGPinProperties> UNGetJunctionDataSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	
	PinProperties.Emplace(JunctionAttribute, EPCGDataType::Point);
	
	return PinProperties;
}

FPCGElementPtr UNGetJunctionDataSettings::CreateElement() const
{
	return MakeShared<FNGetJunctionDataElement>();
}

bool FNGetJunctionDataElement::ExecuteInternal(FPCGContext* Context) const
{
	const UNGetJunctionDataSettings* Settings = Context->GetInputSettings<UNGetJunctionDataSettings>();
	const FVector2D SocketSize = UNProcGenSettings::Get()->SocketSize;
	
	TArray<UNCellJunctionComponent*>& Junctions = FNProcGenRegistry::GetCellJunctionComponents();
	
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;
	UPCGPointData* OutputData = NewObject<UPCGPointData>();
	TArray<FPCGPoint>& MutablePoints = OutputData->GetMutablePoints();
	
	FPCGMetadataAttribute<FVector2D>* SocketSizeAtr = OutputData->Metadata->FindOrCreateAttribute<FVector2D>(TEXT("SocketSize"),FVector2D::ZeroVector, false, true);
	FPCGMetadataAttribute<FVector2D>* WorldSizeAttr = OutputData->Metadata->FindOrCreateAttribute<FVector2D>(TEXT("WorldSize"), FVector2D::ZeroVector, false, true);
	FPCGMetadataAttribute<FRotator>* CardinalRotationAttr = OutputData->Metadata->FindOrCreateAttribute<FRotator>(TEXT("CardinalRotation"), FRotator::ZeroRotator, false, true);
       
	for (const UNCellJunctionComponent* JunctionComponent : Junctions)
	{
		FPCGPoint& NewPoint = MutablePoints.Emplace_GetRef();
		OutputData->Metadata->InitializeOnSet(NewPoint.MetadataEntry);
		
		// World Location
		NewPoint.Transform.SetLocation(JunctionComponent->Details.RootRelativeLocation);
		
		// World Rotation
		NewPoint.Transform.SetRotation(JunctionComponent->Details.RootRelativeCardinalRotation.ToRotator().Quaternion());
		
		// Socket Size
		SocketSizeAtr->SetValue(NewPoint.MetadataEntry, JunctionComponent->Details.SocketSize);
		
		// World Size
		FVector2D WorldSize = FVector2D(
			JunctionComponent->Details.SocketSize.X * SocketSize.X, 
			JunctionComponent->Details.SocketSize.Y * SocketSize.Y);
		WorldSizeAttr->SetValue(NewPoint.MetadataEntry, WorldSize);
		
		// TODO: Optional PACK?
		// Cardinal Rotation
		CardinalRotationAttr->SetValue(NewPoint.MetadataEntry, FRotator(
				static_cast<int>(JunctionComponent->Details.RootRelativeCardinalRotation.Pitch), 
				static_cast<int>(JunctionComponent->Details.RootRelativeCardinalRotation.Yaw), 
				static_cast<int>(JunctionComponent->Details.RootRelativeCardinalRotation.Roll)));
		
		// Bounds
		NewPoint.BoundsMin = FVector(WorldSize.X, Settings->GetParams.SocketDepth, WorldSize.Y) * -0.5f;
		NewPoint.BoundsMax = FVector(WorldSize.X, (JunctionComponent->Details.Type == ENCellJunctionType::TwoWaySocket) ? Settings->GetParams.SocketDepth : 0, WorldSize.Y) * 0.5f;
		
		NewPoint.Seed = 0;
	}
	
	FPCGTaggedData& TaggedData = Outputs.Emplace_GetRef();
	TaggedData.Data = OutputData;
	TaggedData.Pin = Settings->JunctionAttribute; 
	
	
	return true;
}

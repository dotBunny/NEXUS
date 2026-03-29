// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#define N_PCG_JUNCTION_PREFIX \
	const FVector2D SocketSize = UNProcGenSettings::Get()->SocketSize; \
	const float SocketDepth = UNProcGenSettings::Get()->SocketDepth; \
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData; \
	UPCGPointData* OutputData = NewObject<UPCGPointData>(); \
	TArray<FPCGPoint>& MutablePoints = OutputData->GetMutablePoints(); \
	FPCGMetadataAttribute<FVector2D>* SocketSizeAtr = OutputData->Metadata->FindOrCreateAttribute<FVector2D>(TEXT("SocketSize"),FVector2D::ZeroVector, false, true); \
	FPCGMetadataAttribute<FVector2D>* WorldSizeAttr = OutputData->Metadata->FindOrCreateAttribute<FVector2D>(TEXT("WorldSize"), FVector2D::ZeroVector, false, true); \
	FPCGMetadataAttribute<FRotator>* CardinalRotationAttr = OutputData->Metadata->FindOrCreateAttribute<FRotator>(TEXT("CardinalRotation"), FRotator::ZeroRotator, false, true);

#define N_PCG_JUNCTION_DATA \
	FPCGPoint& NewPoint = MutablePoints.Emplace_GetRef(); \
	OutputData->Metadata->InitializeOnSet(NewPoint.MetadataEntry); \
	NewPoint.Transform.SetLocation(JunctionComponent->Details.RootRelativeLocation); \
	NewPoint.Transform.SetRotation(JunctionComponent->Details.RootRelativeCardinalRotation.ToRotator().Quaternion()); \
	SocketSizeAtr->SetValue(NewPoint.MetadataEntry, JunctionComponent->Details.SocketSize); \
	FVector2D WorldSize = FVector2D(JunctionComponent->Details.SocketSize.X * SocketSize.X, JunctionComponent->Details.SocketSize.Y * SocketSize.Y); \
	WorldSizeAttr->SetValue(NewPoint.MetadataEntry, WorldSize); \
	CardinalRotationAttr->SetValue(NewPoint.MetadataEntry, FRotator(static_cast<int>(JunctionComponent->Details.RootRelativeCardinalRotation.Pitch), static_cast<int>(JunctionComponent->Details.RootRelativeCardinalRotation.Yaw), static_cast<int>(JunctionComponent->Details.RootRelativeCardinalRotation.Roll))); \
	NewPoint.BoundsMin = FVector(WorldSize.X, SocketDepth, WorldSize.Y) * -0.5f; \
	NewPoint.BoundsMax = FVector(WorldSize.X, (JunctionComponent->Details.Type == ENCellJunctionType::TwoWaySocket) ? SocketDepth : 0, WorldSize.Y) * 0.5f; \
	NewPoint.Seed = 0;
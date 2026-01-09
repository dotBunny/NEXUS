// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellJunctionComponent.h"

#include "NProcGenRegistry.h"
#include "Cell/NCellRootComponent.h"
#include "NLevelUtils.h"
#include "NProcGenDebugDraw.h"
#include "NProcGenMinimal.h"
#include "NProcGenSettings.h"
#include "NProcGenUtils.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "LevelInstance/LevelInstanceInterface.h"
#include "Math/NVectorUtils.h"

#if WITH_EDITOR
FString UNCellJunctionComponent::GetJunctionName() const
{
	FString ReturnString = GetOwner()->GetActorLabel();
	
	TArray<USceneComponent*> ParentComponents;
	GetParentComponents(ParentComponents);
	for (USceneComponent* Parent : ParentComponents)
	{
		ReturnString.Append(" > ");
		ReturnString.Append(Parent->GetName());
	}

	// Get actual name of the component
	ReturnString.Append(" > ");
	ReturnString.Append(GetName());
	
	return ReturnString;
}
#endif // WITH_EDITOR

FRotator UNCellJunctionComponent::GetOffsetRotator() const
{
	if (LevelInstance != nullptr)
	{
		return LevelInstance->GetActorRotation();
	}
	return FRotator::ZeroRotator;
}

FVector UNCellJunctionComponent::GetOffsetLocation() const
{
	if (LevelInstance != nullptr)
	{
		return LevelInstance->GetActorLocation();
	}
	return FVector::ZeroVector;
}

void UNCellJunctionComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI) const
{
	const UNCellRootComponent* RootComponent = FNProcGenRegistry::GetCellRootComponentFromLevel(GetComponentLevel());
	if (RootComponent == nullptr) return;

	const FVector RootLocation = RootComponent->GetOffsetLocation();
	const FRotator RootRotation = RootComponent->GetOffsetRotator();
	
	const FVector Location =  FNVectorUtils::RotateAndOffsetPoint(this->Details.RootRelativeLocation, RootRotation, RootLocation);

	const FRotator Rotation = Details.RootRelativeCardinalRotation.ToRotatorNormalized() + RootRotation;
	const UNProcGenSettings* Settings = UNProcGenSettings::Get();
	const FVector2D Size = FNProcGenUtils::GetWorldSize2D(Details.UnitSize, Settings->UnitSize);
	const TArray<FVector2D> NubPoints = FNProcGenUtils::GetCenteredWorldPoints2D(Details.UnitSize, Settings->UnitSize);
	
	// Create a 90-degree yaw rotation for the box to render so that it gives a better representation
	const FRotator JunctionRotator = (Rotation.Quaternion() *
		FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(90))).Rotator();

	const TArray<FVector> Points = FNProcGenUtils::GetCenteredWorldCornerPoints2D(Location, JunctionRotator, Size.X,Size.Y, ENAxis::Z);

	const FLinearColor Color = GetColor();
	
	FNProcGenDebugDraw::DrawJunctionRectangle(PDI, Points, Color);
	FNProcGenDebugDraw::DrawJunctionUnits(PDI, Location, JunctionRotator, NubPoints,  Color);

	const float LineLength = Settings->UnitSize.X * 0.25f;

	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Location, Rotation, Color, Details.Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[0], Rotation, Color, Details.Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[1], Rotation, Color, Details.Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[2], Rotation, Color, Details.Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[3], Rotation, Color, Details.Type, LineLength);
}

void UNCellJunctionComponent::OnRegister()
{
	// Is this part of a level instance?
	if (ILevelInstanceInterface* Interface = FNLevelUtils::GetActorComponentLevelInstance(this))
	{
		LevelInstance = Cast<ALevelInstance>(Interface);
	}

	
#if WITH_EDITOR
	const ULevel* Level = GetComponentLevel();
	
	const UNCellRootComponent* RootComponent = FNProcGenRegistry::GetCellRootComponentFromLevel(Level);
	if (RootComponent == nullptr)
	{
		UE_LOG(LogNexusProcGen, Error, TEXT("No UNCellRootComponent found for ULevel(%s); removing added UNCellJunctionComponent next update."), *Level->GetName())
		Level->GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
		{
			this->DestroyComponent();
		}));
	}
	
	ANCellActor* Actor = FNProcGenUtils::GetCellActorFromLevel(Level);
	if (Actor != nullptr && !Actor->WasSpawnedFromProxy())
	{
		// Whilst in the editor we want to make sure that we uniquely identify our junctions
		if (Details.InstanceIdentifier == -1)
		{
			Actor->Modify();
			Details.InstanceIdentifier = Actor->GetCellJunctionNextIdentifier();
		
			Actor->SetActorDirty();
		}
		if (!Actor->CellJunctions.Contains(Details.InstanceIdentifier))
		{
			Actor->Modify();
			Actor->CellJunctions.Add(Details.InstanceIdentifier, this);
		}
	}
#endif
	
	FNProcGenRegistry::RegisterCellJunctionComponent(this);
	Super::OnRegister();
}

void UNCellJunctionComponent::OnUnregister()
{
	ANCellActor* Actor = FNProcGenUtils::GetCellActorFromLevel(GetComponentLevel());
	if (Actor != nullptr && Actor->CellJunctions.Contains(Details.InstanceIdentifier))
	{
		Actor->CellJunctions.Remove(Details.InstanceIdentifier);
	}
	FNProcGenRegistry::UnregisterCellJunctionComponent(this);
	Super::OnUnregister();
}

FLinearColor UNCellJunctionComponent::GetColor() const
{
	switch (Details.Requirements)
	{
		using enum ENCellJunctionRequirements;
	case CJR_AllowBlocking:
		return FNColor::GreenMid;
	case CJR_AllowEmpty:
		return FNColor::GreenDark;
	case CJR_Required:
	default:
		return FNColor::GreenLight;
	}
}

#if WITH_EDITOR
void UNCellJunctionComponent::OnTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	const UNCellRootComponent* RootComponent = FNProcGenRegistry::GetCellRootComponentFromLevel(GetComponentLevel());
	if (RootComponent != nullptr && !RootComponent->GetNCellActor()->WasSpawnedFromProxy())
	{
		bool bHasMadeChanges = false;
		const UNProcGenSettings* Settings = UNProcGenSettings::Get();
		
		// LOCATION
		const FVector ComponentLocation = GetComponentLocation();
		if (const FVector GridLocation = FNVectorUtils::GetClosestGridIntersection(ComponentLocation, Settings->UnitSize);
			GridLocation != Details.RootRelativeLocation)
		{
			// We do not try to store anything about the voxel/final location here as the bounds of the data can change
			Details.RootRelativeLocation = GridLocation;

			bHasMadeChanges = true;
		}

		// ROTATOR
		
		// There should be no root rotation as we haven't been spawned.
		FRotator StartRotator = GetComponentRotation();
		StartRotator.Normalize();
		FRotator FinalRotator = FNCardinalDirectionUtils::GetClosestCardinalRotator(StartRotator);
		FinalRotator.Normalize();
		const FNCardinalRotation CardinalRotation = FNCardinalRotation::CreateFromNormalized(FinalRotator);
		if (!Details.RootRelativeCardinalRotation.IsEqual(CardinalRotation))
		{
			Details.RootRelativeCardinalRotation = CardinalRotation;
			bHasMadeChanges = true;
		}
		
		// Have we made changes, let the people know!
		if (bHasMadeChanges)
		{
			// ReSharper disable once CppExpressionWithoutSideEffects
			MarkPackageDirty();
		}
	}
}


void UNCellJunctionComponent::PostEditImport()
{
	// Forces the instance identifier
	Details.InstanceIdentifier = -1;
}

void UNCellJunctionComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	N_WORLD_ICON_CLEANUP(bDestroyingHierarchy)
}

#endif // WITH_EDITOR

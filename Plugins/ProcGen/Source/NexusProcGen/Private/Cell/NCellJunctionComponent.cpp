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
#include "Cell/NCellLevelInstance.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "LevelInstance/LevelInstanceInterface.h"
#include "Math/NVectorUtils.h"
#include "Types/NRawMeshUtils.h"

#if WITH_EDITOR
FString UNCellJunctionComponent::GetJunctionName() const
{
	FString ReturnString = GetOwner()->GetActorLabel();
	
	TArray<USceneComponent*> ParentComponents;
	GetParentComponents(ParentComponents);
	for (const USceneComponent* Parent : ParentComponents)
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

TArray<FVector> UNCellJunctionComponent::GetCornerPoints(const FVector2D& SocketUnitSize) const
{

	const TArray<FVector> UnrotatedCornerPoints = FNProcGenUtils::GetCenteredWorldCornerPoints2D(
		this->Details.SocketSize.X * SocketUnitSize.X,this->Details.SocketSize.Y * SocketUnitSize.Y, ENAxis::Z);

	// Compose with quaternions; rotator addition only matches composition for yaw-only rotations.
	const FQuat DisplayQuat = FQuat(GetComponentRotation()) * FQuat(FRotator(0.0f, 90.0f, 0.0f));
	const FRotator DisplayRotation = DisplayQuat.Rotator();
	const TArray<FVector> RotatedCornerPoints = FNVectorUtils::RotateAndOffsetPoints(UnrotatedCornerPoints, DisplayRotation, GetComponentLocation());

	return RotatedCornerPoints;
}


void UNCellJunctionComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI) const
{
	const UNProcGenSettings* Settings = UNProcGenSettings::Get();
	FNProcGenDebugDraw::DrawSocket(PDI, GetComponentLocation(), GetComponentRotation(), Details.SocketSize, Settings->SocketSize, Details.Type, GetColor());
}

void UNCellJunctionComponent::OnRegister()
{
	// Is this part of a level instance?
	ILevelInstanceInterface* Interface = FNLevelUtils::GetActorComponentLevelInstance(this);
	if (Interface != nullptr)
	{
		LevelInstance = Cast<ALevelInstance>(Interface);
	}
	
	const ULevel* Level = GetComponentLevel();
	ANCellActor* Actor = FNProcGenUtils::GetCellActorFromLevel(Level);

#if WITH_EDITOR
	// Delay check for root component by a frame, and then will remove the frame after that if not found
	Level->GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
	{
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
	}));
	
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

	
#endif // WITH_EDITOR
	
	// Update details based on generation.
	if (Actor != nullptr && Actor->WasSpawnedFromProxy() && LevelInstance->IsA<ANCellLevelInstance>())
	{
		const ANCellLevelInstance* CellLevelInstance = Cast<ANCellLevelInstance>(LevelInstance);
		const FNCellJunctionDetails* UpdatedDetails = CellLevelInstance->JunctionData.Find(Details.InstanceIdentifier);
		if (UpdatedDetails != nullptr)
		{
			// Copy details in-place
			Details = *UpdatedDetails;

			// Update the rotation so the thing draws nicely; this feels like a bug. The ALevelInstance is supposed
			// to rotate the UWorlds content when it gets placed and loaded. The documentation around the methods seem to
			// infer however that some of this might be editor time only. Not exactly sure what is happening here leading 
			// to the world rotations needing to be updated manually to match the data-only version that we use during
			// generating our NProcGenGraph.
			SetWorldRotation(Details.WorldRotation, false, nullptr, ETeleportType::ResetPhysics);
		}
	}
	
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
	if (Details.bInsideHull)
	{
		return FNColor::Pink;
	}
	
	switch (Details.Requirements)
	{
		using enum ENCellJunctionRequirements;
	case AllowBlocking:
		return FNColor::GreenMid;
	case AllowEmpty:
		return FNColor::GreenDark;
	case Required:
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
		
		// LOCATION
		const FVector ComponentLocation = GetComponentLocation();
		if (ComponentLocation != Details.WorldLocation)
		{
			// We do not try to store anything about the voxel/final location here as the bounds of the data can change
			Details.WorldLocation = ComponentLocation;
			bHasMadeChanges = true;
		}

		// ROTATOR
		const FRotator ComponentRotation = GetComponentRotation();
		if (ComponentRotation != Details.WorldRotation)
		{
			Details.WorldRotation = ComponentRotation;
			bHasMadeChanges = true;
		}

		// Have we made changes, let the people know!
		if (bHasMadeChanges)
		{
			UpdateHullDerivedData(RootComponent);
			
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

void UNCellJunctionComponent::UpdateHullDerivedData(const UNCellRootComponent* RootComponent)
{
	const UNProcGenSettings* Settings = UNProcGenSettings::Get();
	const TArray<FVector> RotatedCornerPoints = GetCornerPoints(Settings->SocketSize);
	
	DrawDebugPoint(GetWorld(), RotatedCornerPoints[0], 10.f, FColor::Red, false, 0.5f);
	DrawDebugPoint(GetWorld(), RotatedCornerPoints[1], 10.f, FColor::Red, false, 0.5f);
	DrawDebugPoint(GetWorld(), RotatedCornerPoints[2], 10.f, FColor::Red, false, 0.5f);
	DrawDebugPoint(GetWorld(), RotatedCornerPoints[3], 10.f, FColor::Red, false, 0.5f);
	
	const bool bIsInside = FNRawMeshUtils::AnyRelativePointsInside(
		RootComponent->Details.Hull, 
		RotatedCornerPoints);
	
	if (Details.bInsideHull != bIsInside)
	{
		Details.bInsideHull = bIsInside;

		// ReSharper disable once CppExpressionWithoutSideEffects
		MarkPackageDirty();
	}
}
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellJunctionComponent.h"

#include "NWorldAssemblyRegistry.h"
#include "Cell/NCellRootComponent.h"
#include "NLevelUtils.h"
#include "NWorldAssemblyDebugDraw.h"
#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblySettings.h"
#include "NWorldAssemblyUtils.h"
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
	if (ALevelInstance* Instance = LevelInstance.Get())
	{
		return Instance->GetActorRotation();
	}
	return FRotator::ZeroRotator;
}

FVector UNCellJunctionComponent::GetOffsetLocation() const
{
	if (ALevelInstance* Instance = LevelInstance.Get())
	{
		return Instance->GetActorLocation();
	}
	return FVector::ZeroVector;
}

TArray<FVector> UNCellJunctionComponent::GetCornerPoints(const FVector2D& SocketUnitSize) const
{

	const TArray<FVector> UnrotatedCornerPoints = FNWorldAssemblyUtils::GetCenteredWorldCornerPoints2D(
		this->Details.SocketSize.X * SocketUnitSize.X,this->Details.SocketSize.Y * SocketUnitSize.Y, ENAxis::Z);

	// Compose with quaternions; rotator addition only matches composition for yaw-only rotations.
	const FQuat DisplayQuat = FQuat(GetComponentRotation()) * FQuat(FRotator(0.0f, 90.0f, 0.0f));
	const FRotator DisplayRotation = DisplayQuat.Rotator();
	const TArray<FVector> RotatedCornerPoints = FNVectorUtils::RotateAndOffsetPoints(UnrotatedCornerPoints, DisplayRotation, GetComponentLocation());

	return RotatedCornerPoints;
}


void UNCellJunctionComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI) const
{
	const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();
	FNWorldAssemblyDebugDraw::DrawSocket(PDI, GetComponentLocation(), GetComponentRotation(), Details.SocketSize, Settings->SocketSize, Details.Type, GetColor());
}

void UNCellJunctionComponent::OnRegister()
{
#if WITH_EDITOR
	// Ensure that undo system works
	SetFlags(RF_Transactional);
#endif
	
	// Is this part of a level instance?
	ILevelInstanceInterface* Interface = FNLevelUtils::GetActorComponentLevelInstance(this);
	if (Interface != nullptr)
	{
		LevelInstance = Cast<ALevelInstance>(Interface);
	}
	
	const ULevel* Level = GetComponentLevel();
	ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromLevel(Level);

#if WITH_EDITOR
	// Author-time validation: catch junctions placed into a level that has no cell root. Skipped for
	// junctions streamed in as part of a cell level instance, where AddToWorld can chunk component
	// registration across ticks and race this check.
	if (!LevelInstance.IsValid())
	{
		TWeakObjectPtr WeakJunctionComponent(this);
		Level->GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([WeakJunctionComponent]()
		{
			if (!WeakJunctionComponent.IsValid()) return;

			const ULevel* Level = WeakJunctionComponent.Get()->GetComponentLevel();
			const UNCellRootComponent* RootComponent = FNWorldAssemblyRegistry::GetCellRootComponentFromLevel(Level);
			if (RootComponent == nullptr)
			{
				UE_LOG(LogNexusWorldAssembly, Error, TEXT("No UNCellRootComponent found for ULevel(%s); removing added UNCellJunctionComponent next update. [registered roots=%d]"),
					*Level->GetPathName(),
					FNWorldAssemblyRegistry::GetCellRootComponents().Num());
				Level->GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([WeakJunctionComponent]()
				{
					if (!WeakJunctionComponent.IsValid()) return;
					WeakJunctionComponent.Get()->DestroyComponent();
				}));
			}
		}));
	}
	
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
			Actor->SetActorDirty();
		}
	}

	
#endif // WITH_EDITOR
	
	// Update details based on generation.
	if (ALevelInstance* Instance = LevelInstance.Get();
		Actor != nullptr && Actor->WasSpawnedFromProxy() && Instance != nullptr && Instance->IsA<ANCellLevelInstance>())
	{
		const ANCellLevelInstance* CellLevelInstance = Cast<ANCellLevelInstance>(Instance);
		const FNCellJunctionDetails* UpdatedDetails = CellLevelInstance->JunctionData.Find(Details.InstanceIdentifier);
		if (UpdatedDetails != nullptr)
		{
			// Copy details in-place
			Details = *UpdatedDetails;

			// Update the rotation so the thing draws nicely; this feels like a bug. The ALevelInstance is supposed
			// to rotate the UWorlds content when it gets placed and loaded. The documentation around the methods seem to
			// infer however that some of this might be editor time only. Not exactly sure what is happening here leading 
			// to the world rotations needing to be updated manually to match the data-only version that we use during
			// generating our FNAssemblyGraph.
			
			// There is some explicit logic around WorldPartition moving everything to a flat-structure when you add ALevelInstances,
			// this logic doesn't appear to be as exercised in the old-school non-world partition way of building levels.
			// It might be related --- I don't know --- hopefully this information might be useful in the future.
			SetWorldRotation(Details.WorldRotation, false, nullptr, ETeleportType::ResetPhysics);
		}
	}
	
	FNWorldAssemblyRegistry::RegisterCellJunctionComponent(this);
	Super::OnRegister();
}

void UNCellJunctionComponent::OnUnregister()
{
	ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromLevel(GetComponentLevel());
	if (Actor != nullptr && Actor->CellJunctions.Contains(Details.InstanceIdentifier))
	{
		Actor->CellJunctions.Remove(Details.InstanceIdentifier);
	}
	FNWorldAssemblyRegistry::UnregisterCellJunctionComponent(this);
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
	const UNCellRootComponent* RootComponent = FNWorldAssemblyRegistry::GetCellRootComponentFromLevel(GetComponentLevel());
	if (RootComponent != nullptr && !RootComponent->GetNCellActor()->WasSpawnedFromProxy())
	{
		bool bHasMadeChanges = false;
		
		// LOCATION
		const FVector ComponentLocation = GetComponentLocation();
		if (ComponentLocation != Details.WorldLocation)
		{
			// We do not try to store anything about the voxel/final location here as the bounds of the data can change
			Modify();
			Details.WorldLocation = ComponentLocation;
			bHasMadeChanges = true;
		}

		// ROTATOR
		const FRotator ComponentRotation = GetComponentRotation();
		if (ComponentRotation != Details.WorldRotation)
		{
			Modify();
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

#endif // WITH_EDITOR

void UNCellJunctionComponent::UpdateHullDerivedData(const UNCellRootComponent* RootComponent)
{
	const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();
	const TArray<FVector> RotatedCornerPoints = GetCornerPoints(Settings->SocketSize);
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

TArray<FVector> UNCellJunctionComponent::GetWorldCornerPoints(const FVector2D& SocketSize) const
{
	const FQuat DisplayQuat = FQuat(GetComponentRotation()) * FQuat(FRotator(0.0f, 90.0f, 0.0f));
	const FRotator DisplayRotation = DisplayQuat.Rotator();
	
	// TODO: Should this maybe be cached at spawning at runtime? 
	const FVector2D Size = FNWorldAssemblyUtils::GetWorldSize2D(Details.SocketSize, SocketSize);

	const TArray<FVector> UnrotatedCornerPoints = FNWorldAssemblyUtils::GetCenteredWorldCornerPoints2D(Size.X,Size.Y, ENAxis::Z);
	const TArray<FVector> RotatedCornerPoints = FNVectorUtils::RotateAndOffsetPoints(UnrotatedCornerPoints, DisplayRotation, GetComponentLocation());
	return RotatedCornerPoints;
}

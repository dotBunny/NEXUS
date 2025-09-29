// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellJunctionComponent.h"

#include "Cell/NCellRegistry.h"
#include "Cell/NCellRootComponent.h"
#include "NLevelUtils.h"
#include "NProcGenDebugDraw.h"
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
	const UNCellRootComponent* RootComponent = FNCellRegistry::GetRootComponentFromLevel(GetComponentLevel());

	const FVector RootLocation = RootComponent->GetOffsetLocation();
	const FRotator RootRotation = RootComponent->GetOffsetRotator();
	
	const FVector Location =  FNVectorUtils::RotateAndOffsetVector(this->Details.RootRelativeLocation, RootRotation, RootLocation);
	const FRotator Rotation = Details.RootRelativeCardinalRotation.ToRotatorNormalized() + RootRotation;
	
	const FVector2D Size = FNProcGenUtils::GetWorldSize2D(Details.Size);
	
	// Create a 90-degree yaw rotation for the box to render so that it gives a better representation
	const FRotator JunctionRotator = (Rotation.Quaternion() *
		FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(90))).Rotator();
	
	FNProcGenDebugDraw::DrawRectangle(PDI, Location, JunctionRotator, Size.X, Size.Y, FLinearColor::Green);

	// We're going to draw some lines to show possible connections
	switch (Details.Type)
	{
	case ENCellJunctionType::NCJT_TwoWaySocket:
		const FVector TwoWayPointA = Location + (Rotation.Vector() * 50.0f);
		const FVector TwoWayPointB = Location + (Rotation.Vector() * -50.0f);
		PDI->DrawLine(TwoWayPointA, TwoWayPointB, FLinearColor::Green, SDPG_World);
		PDI->DrawPoint(TwoWayPointA, FLinearColor::Green, 10.0f, SDPG_World);
		PDI->DrawPoint(TwoWayPointB, FLinearColor::Green, 10.0f, SDPG_World);
		break;
	case ENCellJunctionType::NCJT_OneWaySocket:
		const FVector OneWayPoint = Location + (Rotation.Vector() * 50.0f);
		PDI->DrawLine(Location, OneWayPoint, FLinearColor::Green, SDPG_World);
		PDI->DrawPoint(OneWayPoint, FLinearColor::Green, 10.0f, SDPG_World);
		break;
	}
}

void UNCellJunctionComponent::OnRegister()
{
	// Is this part of a level instance?
	if (ILevelInstanceInterface* Interface = FNLevelUtils::GetActorComponentLevelInstance(this))
	{
		LevelInstance = Cast<ALevelInstance>(Interface);
	}

	ANCellActor* Actor = FNProcGenUtils::GetNCellActorFromLevel(GetComponentLevel());
#if WITH_EDITOR
	// Whilst in the editor we want to make sure that we uniquely identify our junctions
	if (Details.InstanceIdentifier == -1)
	{
		Details.InstanceIdentifier = Actor->GetCellJunctionNextIdentifier();
		Actor->SetActorDirty();
	}
#endif
	if (!Actor->CellJunctions.Contains(Details.InstanceIdentifier))
	{
		Actor->CellJunctions.Add(Details.InstanceIdentifier, this);
	}
	
	FNCellRegistry::RegisterJunctionComponent(this);
	Super::OnRegister();
}

void UNCellJunctionComponent::OnUnregister()
{
	ANCellActor* Actor = FNProcGenUtils::GetNCellActorFromLevel(GetComponentLevel());
	if (Actor != nullptr && Actor->CellJunctions.Contains(Details.InstanceIdentifier))
	{
		Actor->CellJunctions.Remove(Details.InstanceIdentifier);
	}
	FNCellRegistry::UnregisterJunctionComponent(this);
	Super::OnUnregister();
}

#if WITH_EDITOR
void UNCellJunctionComponent::OnTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	const UNCellRootComponent* RootComponent = FNCellRegistry::GetRootComponentFromLevel(GetComponentLevel());
	if (RootComponent != nullptr && !RootComponent->GetNCellActor()->WasSpawnedFromProxy())
	{
		bool bHasMadeChanges = false;
		
		// There should be no root rotation as we haven't been spawned.
		FRotator StartRotator = GetComponentRotation();

		// Let us not go crazy
		StartRotator.Normalize();
		
		FRotator FinalRotator = FNCardinalDirectionUtils::GetClosestCardinalRotator(StartRotator);
		FinalRotator.Normalize();
		
		if (StartRotator != FinalRotator)
		{
			SetWorldRotation(FinalRotator);
			bHasMadeChanges = true;
		}
		
		// Update component cache
		if (const FVector ComponentLocation = GetComponentLocation();
			ComponentLocation != Details.RootRelativeLocation)
		{
			Details.RootRelativeLocation = ComponentLocation;
			bHasMadeChanges = true;
		}
		
		// Update our rotation
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

void UNCellJunctionComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	N_WORLD_ICON_CLEANUP()
}

#endif // WITH_EDITOR

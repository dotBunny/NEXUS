// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NBoneComponent.h"

#include "NProcGenDebugDraw.h"
#include "NProcGenMinimal.h"
#include "NProcGenRegistry.h"
#include "NProcGenSettings.h"
#include "NProcGenUtils.h"
#include "Components/BillboardComponent.h"
#include "Components/BrushComponent.h"
#include "Macros/NActorMacros.h"
#include "Math/NVectorUtils.h"
#include "Organ/NBoneActor.h"
#include "Organ/NOrganComponent.h"

namespace NEXUS::ProcGen::Bone
{
	const FName Mode = FName("Mode");
}

UNBoneComponent::UNBoneComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Mobility = EComponentMobility::Static;

#if WITH_EDITOR
	TransformUpdated.AddUObject(this, &UNBoneComponent::OnTransformUpdated);
#endif
	
	N_WORLD_ICON_IMPLEMENTATION_SCENE_COMPONENT("/NexusProcGen/EditorResources/S_NBoneComponent", this, false, 0.35f)
}

#if WITH_EDITOR

void UNBoneComponent::OnRegister()
{
	const ULevel* Level = GetComponentLevel();
	if (const UNCellRootComponent* RootComponent = FNProcGenRegistry::GetCellRootComponentFromLevel(Level); 
		RootComponent != nullptr)
	{
		UE_LOG(LogNexusProcGen, Error, TEXT("You cannot place UNBoneComponent in a ULevel(%s) where an NCellRootComponent is defined; removing next update."), *Level->GetName())
		Level->GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
		{
			if (AActor* Actor = this->GetOwner(); Actor != nullptr)
			{
				if (ANBoneActor* BoneActor = Cast<ANBoneActor>(Actor); BoneActor != nullptr)
				{
					BoneActor->Destroy();
					return;
				}
			}
			this->DestroyComponent();
		}));
	}

	FNProcGenRegistry::RegisterBoneComponent(this);
	Super::OnRegister();
}

void UNBoneComponent::OnUnregister()
{
	FNProcGenRegistry::UnregisterBoneComponent(this);
	Super::OnUnregister();
}

void UNBoneComponent::OnTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateTransformFlags,
                                         ETeleportType Teleport)
{
	if (Mode == ENBoneMode::Automatic)
	{
		SetAutomaticTransform();
		return;
	}
	
	// LOCATION
	const FVector BoneLocation = GetComponentLocation();
	const FVector WorkingLocation = FindSafeLocation(BoneLocation);
	if (WorkingLocation != BoneLocation)
	{
		Mode = ENBoneMode::Manual;
		SetWorldLocation(WorkingLocation);
		// ReSharper disable once CppExpressionWithoutSideEffects
		MarkPackageDirty();
	}
}

void UNBoneComponent::SetAutomaticTransform()
{
	if (Mode != ENBoneMode::Automatic || OrganComponent == nullptr || !OrganComponent->IsVolumeBased()) return;
	
	const UNProcGenSettings* Settings = UNProcGenSettings::Get();
	if (Settings == nullptr) return;
	
	const AVolume* OrganVolume = Cast<AVolume>(OrganComponent->GetOwner());
	const FVector BoneLocation = GetComponentLocation();
		
	const FVector OrganBoneDirection = FNDirection::GetVector(Settings->OrganAutomaticBoneDirection);
	const FVector OutsidePoint = 
		OrganVolume->GetBounds().Origin + 
		((OrganBoneDirection + Settings->OrganAutomaticBoneDirectionOffset).GetSafeNormal() * (OrganVolume->GetBounds().SphereRadius));
		
	UBrushComponent* BrushComponent = OrganVolume->GetBrushComponent();
	if (BrushComponent == nullptr)
	{
		return;
	}
	
	FVector HitLocation;
	FVector HitNormal;
	FName HitBone;
	FHitResult HitResult;
		
	if (BrushComponent->K2_LineTraceComponent(OutsidePoint, OrganVolume->GetBounds().Origin, 
		true, false, false, 
		HitLocation, HitNormal, HitBone, HitResult))
	{
		const FRotator UpdatedRotator = OrganBoneDirection.ToOrientationRotator() + FRotator(0,-90,0);
		if (GetComponentRotation() != UpdatedRotator)
		{
			SetWorldRotation(UpdatedRotator);	
				
			// ReSharper disable once CppExpressionWithoutSideEffects
			MarkPackageDirty();
		}
			
		const FVector WorkingPosition = FindSafeLocation(HitLocation);
		if (BoneLocation != WorkingPosition)
		{
			SetWorldLocation(WorkingPosition);
				
			// ReSharper disable once CppExpressionWithoutSideEffects
			MarkPackageDirty();
		}
	}
}


FVector UNBoneComponent::FindSafeLocation(const FVector& WorldLocation) const
{
	FVector WorkingLocation = WorldLocation;
	
	if (OrganComponent == nullptr || !OrganComponent->IsVolumeBased())
	{
		return WorldLocation;
	}
	
	const AVolume* OrganVolume = Cast<AVolume>(OrganComponent->GetOwner());
	const UBrushComponent* BrushComponent = OrganVolume->GetBrushComponent();
	
	if (BrushComponent == nullptr)
	{
		return WorldLocation;
	}
	
	// We have a brush that we can use
	const UNProcGenSettings* Settings = UNProcGenSettings::Get();
	const auto UnitSizeX = static_cast<float>(SocketSize.X * Settings->SocketSize.X);
	const auto UnitSizeY = static_cast<float>(SocketSize.Y * Settings->SocketSize.Y);
	
	// Create a 90-degree yaw rotation for the box that we can use to offset later.
	const FRotator JunctionRotator = (GetComponentQuat() *
	FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(90))).Rotator();
	
	// TODO: THIS NEEDS TO BE FIXED WITH THE NEW ROTATION STUFF 
	TArray<FVector> Points = FNProcGenUtils::GetCenteredWorldCornerPoints2D(UnitSizeX,UnitSizeY, ENAxis::Z);
	TArray<FVector> RotatedPoints = FNVectorUtils::RotateAndOffsetPoints(Points, JunctionRotator, WorldLocation);
	
	int32 IterationCount = 12;
	FVector ClosestLocation;
	bool bDidAdjust = true;
	while (IterationCount > 0 && bDidAdjust)
	{
		bDidAdjust = false;
		for (int i = 0; i < 4; i++)
		{
			const float Distance = BrushComponent->GetClosestPointOnCollision(
				RotatedPoints[i], ClosestLocation, NAME_None);
			
			if (Distance <= 0.f)
			{
				continue;
			}
			// Point is outside we need to now find the closest point and bring it in
			const FVector Delta = ClosestLocation - RotatedPoints[i];
			
			RotatedPoints[0] += Delta;
			RotatedPoints[1] += Delta;
			RotatedPoints[2] += Delta;
			RotatedPoints[3] += Delta;
			
			WorkingLocation += Delta;
			bDidAdjust = true;
			
		}
		IterationCount--;
	}
	
	return WorkingLocation;
}

void UNBoneComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property->GetFName() == NEXUS::ProcGen::Bone::Mode)
	{
		OnModeChanged(Mode);
	}
}

void UNBoneComponent::OnModeChanged(const ENBoneMode NewMode)
{
	switch (NewMode)
	{
		using enum ENBoneMode;
	case Manual:
#if WITH_EDITORONLY_DATA
		SpriteComponent->SetVisibility(true);
#endif // WITH_EDITORONLY_DATA
		if (!IsActive())
		{
			ToggleActive();
		}
		break;
		
	case Automatic:
		SetAutomaticTransform();
#if WITH_EDITORONLY_DATA
		SpriteComponent->SetVisibility(true);
#endif // WITH_EDITORONLY_DATA
		if (!IsActive())
		{
			ToggleActive();
		}
		break;

	case Disabled:
#if WITH_EDITORONLY_DATA
		SpriteComponent->SetVisibility(false);
#endif // WITH_EDITORONLY_DATA
		if (IsActive())
		{
			ToggleActive();
		}
		// ZERO out data?
		break;
	}
}


#endif // WITH_EDITOR


FVector UNBoneComponent::GetMinimumPoint(const FVector& BaseLocation, const FRotator& OffsetRotation, const FVector2D& SocketUnitSize) const
{
	FVector MinimumPoint =
		(FVector(this->SocketSize.X * SocketUnitSize.X, 0, this->SocketSize.Y * SocketUnitSize.Y) * -0.5f);
	
	MinimumPoint = this->GetComponentRotation().RotateVector(MinimumPoint);
	MinimumPoint = OffsetRotation.RotateVector(MinimumPoint);
	MinimumPoint += BaseLocation;

	return MoveTemp(MinimumPoint);
}

FVector UNBoneComponent::GetMaximumPoint(const FVector& BaseLocation, const FRotator& OffsetRotation, const FVector2D& SocketUnitSize) const
{
	FVector MaximumPoint =
			(FVector(this->SocketSize.X * SocketUnitSize.X, 0, this->SocketSize.Y * SocketUnitSize.Y) * 0.5f);
	
	MaximumPoint = this->GetComponentRotation().RotateVector(MaximumPoint);
	MaximumPoint = OffsetRotation.RotateVector(MaximumPoint);
	MaximumPoint += BaseLocation;

	return MoveTemp(MaximumPoint);
}

FString UNBoneComponent::GetDebugLabel() const
{
	return GetOwner()->GetActorNameOrLabel();
}


void UNBoneComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI) const
{
	FLinearColor Color = FLinearColor::White;
	switch (Mode)
	{
		using enum ENBoneMode;
	case Manual:
		break;
	case Automatic:
		Color = FNColor::NexusLightBlue;
		break;
	case Disabled:
		// Were not drawing this
		return;
	}
	
	const UNProcGenSettings* Settings = UNProcGenSettings::Get();

	FNProcGenDebugDraw::DrawSocket(PDI,  this->GetComponentLocation(), this->GetComponentRotation().GetNormalized(), 
		SocketSize, Settings->SocketSize, Type, Color);
}

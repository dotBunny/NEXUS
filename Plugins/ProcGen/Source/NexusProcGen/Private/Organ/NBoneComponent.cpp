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
	
	FRotator StartRotator = GetComponentRotation();
	StartRotator.Normalize();
	FRotator FinalRotator = FNCardinalDirectionUtils::GetClosestCardinalRotator(StartRotator);
	FinalRotator.Normalize();
	WorldCardinalRotation = FNCardinalRotation::CreateFromNormalized(FinalRotator);
	
	FRotator UpdatedRotator = WorldCardinalRotation.ToRotator();
	if (StartRotator != UpdatedRotator)
	{
		SetWorldRotation(UpdatedRotator);
		// ReSharper disable once CppExpressionWithoutSideEffects
		MarkPackageDirty();
	}
	
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
			WorldCardinalRotation = FNCardinalRotation::CreateFromNormalized(UpdatedRotator);
				
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
	
	TArray<FVector> Points = FNProcGenUtils::GetCenteredWorldCornerPoints2D(WorldLocation, JunctionRotator, UnitSizeX,UnitSizeY, ENAxis::Z);
	int32 IterationCount = 12;
	FVector ClosestLocation;
	bool bDidAdjust = true;
	while (IterationCount > 0 && bDidAdjust)
	{
		bDidAdjust = false;
		for (int i = 0; i < 4; i++)
		{
			const float Distance = BrushComponent->GetClosestPointOnCollision(
				Points[i], ClosestLocation, NAME_None);
			
			if (Distance <= 0.f)
			{
				continue;
			}
			// Point is outside we need to now find the closest point and bring it in
			const FVector Delta = ClosestLocation - Points[i];
			
			Points[0] += Delta;
			Points[1] += Delta;
			Points[2] += Delta;
			Points[3] += Delta;
			
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

void UNBoneComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI) const
{
	const FVector RootLocation = GetComponentLocation();
	const FRotator Rotation = WorldCardinalRotation.ToRotatorNormalized();
	const UNProcGenSettings* Settings = UNProcGenSettings::Get();
	const FVector2D Size = FNProcGenUtils::GetWorldSize2D(SocketSize, Settings->SocketSize);
	const TArray<FVector2D> NubPoints = FNProcGenUtils::GetCenteredWorldPoints2D(SocketSize, Settings->SocketSize);
	
	const TArray<FVector> Points = FNProcGenUtils::GetCenteredWorldCornerPoints2D(RootLocation, Rotation, Size.X,Size.Y, ENAxis::Z);
	
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
		return;
	}
	
	FNProcGenDebugDraw::DrawJunctionRectangle(PDI, Points, Color);
	FNProcGenDebugDraw::DrawJunctionUnits(PDI, RootLocation, Rotation, NubPoints,  Color);

	const float LineLength = Settings->SocketSize.X * 0.25f;
	
	// Create a 90-degree yaw rotation for the box to render so that it gives a better representation
	const FRotator SocketTypeRotator = (Rotation.Quaternion() *
		FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(90))).Rotator();
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, RootLocation, SocketTypeRotator, Color, Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[0], SocketTypeRotator, Color, Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[1], SocketTypeRotator, Color, Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[2], SocketTypeRotator, Color, Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[3], SocketTypeRotator, Color, Type, LineLength);
}

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



void UNBoneComponent::OnRegister()
{

#if WITH_EDITOR
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
#endif // WITH_EDITOR	

	FNProcGenRegistry::RegisterBoneComponent(this);
	Super::OnRegister();
}

void UNBoneComponent::OnUnregister()
{
	FNProcGenRegistry::UnregisterBoneComponent(this);
	Super::OnUnregister();
}

#if WITH_EDITOR
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
		const FRotator UpdatedRotator = OrganBoneDirection.ToOrientationRotator() + FRotator(0,-180,0);
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
	TArray<FVector> CornerPoints = GetCornerPoints(Settings->SocketSize);
	
	int32 IterationCount = 12;
	FVector ClosestLocation;
	bool bDidAdjust = true;
	while (IterationCount > 0 && bDidAdjust)
	{
		bDidAdjust = false;
		for (int i = 0; i < 4; i++)
		{
			const float Distance = BrushComponent->GetClosestPointOnCollision(
				CornerPoints[i], ClosestLocation, NAME_None);
			
			if (Distance <= 0.f)
			{
				continue;
			}
			// Point is outside we need to now find the closest point and bring it in
			const FVector Delta = ClosestLocation - CornerPoints[i];
			
			CornerPoints[0] += Delta;
			CornerPoints[1] += Delta;
			CornerPoints[2] += Delta;
			CornerPoints[3] += Delta;
			
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

FString UNBoneComponent::GetDebugLabel() const
{
	return GetOwner()->GetActorNameOrLabel();
}

TArray<FVector> UNBoneComponent::GetCornerPoints(const FVector2D& SocketUnitSize) const
{
	const TArray<FVector> UnrotatedCornerPoints = FNProcGenUtils::GetCenteredWorldCornerPoints2D(
		this->SocketSize.X * SocketUnitSize.X,this->SocketSize.Y * SocketUnitSize.Y, ENAxis::Z);

	const FRotator DisplayRotation = GetComponentRotation() + FRotator(0.0f, 90.0f, 0.0f);
	const TArray<FVector> RotatedCornerPoints = FNVectorUtils::RotateAndOffsetPoints(UnrotatedCornerPoints, DisplayRotation, GetComponentLocation());
	
	return RotatedCornerPoints;
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

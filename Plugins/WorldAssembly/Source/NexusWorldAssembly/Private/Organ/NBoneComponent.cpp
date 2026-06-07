// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NBoneComponent.h"

#include "NWorldAssemblyDebugDraw.h"
#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblySettings.h"
#include "NWorldAssemblyUtils.h"
#include "Components/BillboardComponent.h"
#include "Components/BrushComponent.h"
#include "Developer/NPrimitiveFont.h"
#include "Macros/NActorMacros.h"
#include "Math/NVectorUtils.h"
#include "Organ/NBoneActor.h"
#include "Organ/NOrganComponent.h"

namespace NEXUS::WorldAssembly::Bone
{
	inline const FName Mode = FName("Mode");
}

UNBoneComponent::UNBoneComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Mobility = EComponentMobility::Static;

#if WITH_EDITOR
	TransformUpdated.AddUObject(this, &UNBoneComponent::OnTransformUpdated);
#endif
	
	N_WORLD_ICON_SCENE_COMPONENT("/NexusWorldAssembly/EditorResources/S_NBoneComponent", this, false, 0.35f)
}



void UNBoneComponent::OnRegister()
{
#if WITH_EDITOR
	// Ensure that undo system works
	SetFlags(RF_Transactional);
	
	const ULevel* Level = GetComponentLevel();
	TWeakObjectPtr WeakBoneComponent(this);
	if (const UNCellRootComponent* RootComponent = FNWorldAssemblyRegistry::GetCellRootComponentFromLevel(Level); 
		RootComponent != nullptr)
	{
		UE_LOG(LogNexusWorldAssembly, Error, TEXT("You cannot place UNBoneComponent in a ULevel(%s) where an NCellRootComponent is defined; removing next update."), *Level->GetName());
		Level->GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([WeakBoneComponent]()
		{
			// Early out for dead ref
			if (!WeakBoneComponent.IsValid()) return;
			
			if (AActor* Actor = WeakBoneComponent.Get()->GetOwner(); Actor != nullptr)
			{
				if (ANBoneActor* BoneActor = Cast<ANBoneActor>(Actor); BoneActor != nullptr)
				{
					BoneActor->Destroy();
					return;
				}
			}
			WeakBoneComponent.Get()->DestroyComponent();
		}));
	}
#endif // WITH_EDITOR	

	FNWorldAssemblyRegistry::RegisterBoneComponent(this);
	Super::OnRegister();
}

void UNBoneComponent::OnUnregister()
{
	FNWorldAssemblyRegistry::UnregisterBoneComponent(this);
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
	
	const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();
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
	const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();
	TArray<FVector> CornerPoints = GetCornerPoints(Settings->SocketSize);
	
	int32 IterationCount = 12;
	FVector ClosestLocation;
	bool bDidAdjust = true;
	while (IterationCount > 0 && bDidAdjust)
	{
		bDidAdjust = false;
		for (int32 i = 0; i < 4; i++)
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

	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == NEXUS::WorldAssembly::Bone::Mode)
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
	const TArray<FVector> UnrotatedCornerPoints = FNWorldAssemblyUtils::GetCenteredWorldCornerPoints2D(
		this->SocketSize.X * SocketUnitSize.X,this->SocketSize.Y * SocketUnitSize.Y, ENAxis::Z);

	// Compose with quaternions; rotator addition only matches composition for yaw-only rotations.
	const FQuat DisplayQuat = FQuat(GetComponentRotation()) * FQuat(FRotator(0.0f, 90.0f, 0.0f));
	const FRotator DisplayRotation = DisplayQuat.Rotator();
	const TArray<FVector> RotatedCornerPoints = FNVectorUtils::RotateAndOffsetPoints(UnrotatedCornerPoints, DisplayRotation, GetComponentLocation());
	
	return RotatedCornerPoints;
}


void UNBoneComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI, bool bShowDepth, const UNWorldAssemblySettings* Settings, float WorldPenetration) const
{
	FLinearColor GizmoColor = FLinearColor::White;

	switch (Mode)
	{
		using enum ENBoneMode;
	case Manual:
		break;
	case Automatic:
		GizmoColor = FNColor::NexusLightBlue;
		break;
	case Disabled:
		// Were not drawing this
		return;
	}
	
	const FVector ComponentLocation = GetComponentLocation();
	const FRotator ComponentRotation = GetComponentRotation();
	
	if (bShowDepth)
	{
		TArray<FVector> CornerPoints = GetWorldCornerPoints(Settings->SocketSize);

		// Penetration is supplied by the caller (the visualizer samples FNWorldCollisionCache); the component itself
		// holds no world-collision state.
		const float MaximumDepth = WorldPenetration;

		if (MaximumDepth > Settings->AssemblyJunctionMatchingWorldPenetration)
		{
			GizmoColor = FLinearColor::Red;
		}
		
		// Draw the depth text
		if (MaximumDepth != 0)
		{
			// Always draw the readout upright in world space, directly beneath the junction. Using only the junction's
			// yaw (zero pitch/roll) keeps the glyphs world-upright no matter how the junction is oriented, so the text
			// never ends up upside down. Anchoring at the socket's lowest world-Z corner keeps it clear of the socket.

			double LowestZ = ComponentLocation.Z;
			for (const FVector& Corner : CornerPoints)
			{
				LowestZ = FMath::Min(LowestZ, Corner.Z);
			}
			
			const FVector TextPosition(ComponentLocation.X, ComponentLocation.Y, LowestZ - 4.0f);
			const FRotator TextRotation(0.0, ComponentRotation.Yaw, 0.0);

			FNPrimitiveFont::DrawPDI(PDI, FString::Printf(TEXT("%.1f"),MaximumDepth), 
				TextPosition, TextRotation, GizmoColor,0.15f, 1.f, 1.f, 
				false, true, SDPG_Foreground);
		}
	}

	FNWorldAssemblyDebugDraw::DrawSocket(PDI,  ComponentLocation, ComponentRotation, SocketSize, Settings->SocketSize, Type, GizmoColor);
}


TArray<FVector> UNBoneComponent::GetWorldCornerPoints(const FVector2D& SettingSocketSize) const
{
	const FQuat DisplayQuat = FQuat(GetComponentRotation()) * FQuat(FRotator(0.0f, 90.0f, 0.0f));
	const FRotator DisplayRotation = DisplayQuat.Rotator();
	
	// TODO: Should this maybe be cached at spawning at runtime? 
	const FVector2D Size = FNWorldAssemblyUtils::GetWorldSize2D(SocketSize, SettingSocketSize);

	const TArray<FVector> UnrotatedCornerPoints = FNWorldAssemblyUtils::GetCenteredWorldCornerPoints2D(Size.X,Size.Y, ENAxis::Z);
	const TArray<FVector> RotatedCornerPoints = FNVectorUtils::RotateAndOffsetPoints(UnrotatedCornerPoints, DisplayRotation, GetComponentLocation());
	return RotatedCornerPoints;
}


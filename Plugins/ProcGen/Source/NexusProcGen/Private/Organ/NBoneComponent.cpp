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
	Super::OnRegister();
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
		MarkPackageDirty();
		SetWorldRotation(UpdatedRotator);	
	}
	
	
	if (OrganComponent != nullptr && OrganComponent->IsVolumeBased())
	{
		const AVolume* OrganVolume = Cast<AVolume>(OrganComponent->GetOwner());
		const UBrushComponent* BrushComponent = OrganVolume->GetBrushComponent();

		if (BrushComponent != nullptr)
		{
			const UNProcGenSettings* Settings = UNProcGenSettings::Get();
			const float UnitSizeX = static_cast<float>(UnitSize.X * Settings->UnitSize.X);
			const float UnitSizeY = static_cast<float>(UnitSize.Y * Settings->UnitSize.Y);
			
			const FVector BoneLocation = GetComponentLocation();
			FVector WorkingLocation = BoneLocation;
			
			// Create a 90-degree yaw rotation for the box that we can use to offset later.
			const FRotator JunctionRotator = (UpdatedRotator.Quaternion() *
			FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(90))).Rotator();
			
			TArray<FVector> Points = FNProcGenUtils::GetCenteredWorldCornerPoints2D(BoneLocation, JunctionRotator, UnitSizeX,UnitSizeY, ENAxis::Z);
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
					
					// Point is outside we need to now find the closest point and bring it in
					if (Distance > 0.f)
					{
						const FVector Delta = ClosestLocation - Points[i];
						
						Points[0] += Delta;
						Points[1] += Delta;
						Points[2] += Delta;
						Points[3] += Delta;
						
						WorkingLocation += Delta;
						bDidAdjust = true;
					}
				}
				IterationCount--;
			}
			
			if (WorkingLocation != BoneLocation)
			{
				Mode = ENBoneMode::Manual;
				SetWorldLocation(WorkingLocation);
				MarkPackageDirty();
			}
		}
	}
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
	case ENBoneMode::Manual:
#if WITH_EDITORONLY_DATA
		SpriteComponent->SetVisibility(true);
#endif // WITH_EDITORONLY_DATA
		if (!IsActive())
		{
			ToggleActive();
		}
		break;
		
	case ENBoneMode::Automatic:
		SetAutomaticTransform();
#if WITH_EDITORONLY_DATA
		SpriteComponent->SetVisibility(true);
#endif // WITH_EDITORONLY_DATA
		if (!IsActive())
		{
			ToggleActive();
		}
		break;

	case ENBoneMode::Disabled:
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

void UNBoneComponent::SetAutomaticTransform()
{
	if (Mode != ENBoneMode::Automatic || OrganComponent == nullptr ) return;
	
	const UNProcGenSettings* Settings = UNProcGenSettings::Get();
	if (Settings == nullptr) return;
	
	UE_LOG(LogTemp, Warning, TEXT("Setting automatic transform for bone component"));
	
	if (OrganComponent->IsVolumeBased())
	{
		const AVolume* OrganVolume = Cast<AVolume>(OrganComponent->GetOwner());
		const FBoxSphereBounds OrganVolumeBounds = OrganVolume->GetBounds();
		
		//OrganVolumeBounds.GetBox().GetCenter()
		
		
		//Settings->OrganAutomaticBoneDirection;
	}
}

#endif // WITH_EDITOR

void UNBoneComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI) const
{
	const FVector RootLocation = GetComponentLocation();
	const FRotator Rotation = WorldCardinalRotation.ToRotatorNormalized();
	const UNProcGenSettings* Settings = UNProcGenSettings::Get();
	const FVector2D Size = FNProcGenUtils::GetWorldSize2D(UnitSize, Settings->UnitSize);
	const TArray<FVector2D> NubPoints = FNProcGenUtils::GetCenteredWorldPoints2D(UnitSize, Settings->UnitSize);
	
	// Create a 90-degree yaw rotation for the box to render so that it gives a better representation
	 const FRotator BoneVisualizationRotator = (Rotation.Quaternion() *
	 	FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(90))).Rotator();
	
	const TArray<FVector> Points = FNProcGenUtils::GetCenteredWorldCornerPoints2D(RootLocation, BoneVisualizationRotator, Size.X,Size.Y, ENAxis::Z);

	
	FLinearColor Color = FLinearColor::White;
	switch (Mode)
	{
	case ENBoneMode::Manual:
		break;
	case ENBoneMode::Automatic:
		Color = FNColor::NexusLightBlue;
		break;
	case ENBoneMode::Disabled:
		return;
	}
	
	FNProcGenDebugDraw::DrawJunctionRectangle(PDI, Points, Color);
	FNProcGenDebugDraw::DrawJunctionUnits(PDI, RootLocation, BoneVisualizationRotator, NubPoints,  Color);

	const float LineLength = Settings->UnitSize.X * 0.25f;

	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, RootLocation, Rotation, Color, Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[0], Rotation, Color, Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[1], Rotation, Color, Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[2], Rotation, Color, Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[3], Rotation, Color, Type, LineLength);
}

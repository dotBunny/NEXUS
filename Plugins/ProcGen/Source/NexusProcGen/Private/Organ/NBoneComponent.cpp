// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NBoneComponent.h"

#include "NProcGenDebugDraw.h"
#include "NProcGenMinimal.h"
#include "NProcGenRegistry.h"
#include "NProcGenSettings.h"
#include "NProcGenUtils.h"
#include "Components/BillboardComponent.h"
#include "Macros/NActorMacros.h"
#include "Math/NBoundsUtils.h"
#include "Math/NVectorUtils.h"
#include "Organ/NBoneActor.h"
#include "Organ/NOrganComponent.h"

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
	FRotator StartRotator = GetComponentRotation();
	StartRotator.Normalize();
	FRotator FinalRotator = FNCardinalDirectionUtils::GetClosestCardinalRotator(StartRotator);
	FinalRotator.Normalize();
	RootRelativeCardinalRotation = FNCardinalRotation::CreateFromNormalized(FinalRotator);
	
	if (OrganComponent != nullptr && OrganComponent->IsVolumeBased())
	{
		const AVolume* OrganVolume = Cast<AVolume>(OrganComponent->GetOwner());
		const FBoxSphereBounds OrganVolumeBounds = OrganVolume->GetBounds();
		
		const FVector BoneLocation = GetComponentLocation();
		
		const UNProcGenSettings* Settings = UNProcGenSettings::Get();
		const float UnitSizeX = static_cast<float>(UnitSize.X * Settings->UnitSize.X);
		const float UnitSizeY = static_cast<float>(UnitSize.Y * Settings->UnitSize.Y);
		
		// TODO: Need to get offset based on rotation
		// TODO: Need to actual validate the margin stuff is working as expected
		const FVector SizeOffset = FVector(UnitSizeX * 0.5f, UnitSizeY * 0.5f, 0);
		const FVector ClosestPoint = FNBoundsUtils::GetPointInBoundsWithMargin(BoneLocation, OrganVolumeBounds, SizeOffset);
		
		if (ClosestPoint != BoneLocation)
		{
			SetWorldLocation(ClosestPoint);
		}
	}
}
#endif // WITH_EDITOR

void UNBoneComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI) const
{
	const FVector RootLocation = GetComponentLocation();
	const FRotator RootRotation = GetComponentRotation();

	const FRotator Rotation = RootRelativeCardinalRotation.ToRotatorNormalized() + RootRotation;
	const UNProcGenSettings* Settings = UNProcGenSettings::Get();
	const FVector2D Size = FNProcGenUtils::GetWorldSize2D(UnitSize, Settings->UnitSize);
	const TArray<FVector2D> NubPoints = FNProcGenUtils::GetCenteredWorldPoints2D(UnitSize, Settings->UnitSize);
	
	// Create a 90-degree yaw rotation for the box to render so that it gives a better representation
	const FRotator JunctionRotator = (Rotation.Quaternion() *
		FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(90))).Rotator();

	const TArray<FVector> Points = FNProcGenUtils::GetCenteredWorldCornerPoints2D(RootLocation, JunctionRotator, Size.X,Size.Y, ENAxis::Z);

	const FLinearColor Color = FLinearColor::White;
	
	FNProcGenDebugDraw::DrawJunctionRectangle(PDI, Points, Color);
	FNProcGenDebugDraw::DrawJunctionUnits(PDI, RootLocation, JunctionRotator, NubPoints,  Color);

	const float LineLength = Settings->UnitSize.X * 0.25f;

	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, RootLocation, Rotation, Color, Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[0], Rotation, Color, Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[1], Rotation, Color, Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[2], Rotation, Color, Type, LineLength);
	FNProcGenDebugDraw::DrawJunctionSocketTypePoint(PDI, Points[3], Rotation, Color, Type, LineLength);
}

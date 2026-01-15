// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSamplesDisplayBuilder.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

void FNSamplesDisplayBuilder::CreateScalablePanelInstances(const FTransform& BaseTransform, UInstancedStaticMeshComponent* PanelMain, UInstancedStaticMeshComponent* PanelCorner, UInstancedStaticMeshComponent* PanelSide, float Length, float Width, bool bIgnoreMainPanel)
{
	const float PrimaryScaleY = (Width - 2) * 0.5f;
	const float PrimaryScaleZ = (Length - 2) * 0.5f;
	const float SecondaryY = ((Width - 2.f) * .5f) * 100.f;
	const float SecondaryZ = (Length - 2.f) * 100.f;
	const float ScaledLocationY = PrimaryScaleY * 100.f;
	const float ScaledLocationZ = PrimaryScaleZ * 100.f;

	if (!bIgnoreMainPanel)
	{
		PanelMain->AddInstance(
			UKismetMathLibrary::MakeRelativeTransform(
				FTransform(
					FRotator::ZeroRotator,
					FVector::ZeroVector,
					FVector(1.f, PrimaryScaleY, PrimaryScaleZ)),
					BaseTransform), false);
	}
		
	// Corners
	TArray<FTransform> InstanceTransforms;
	InstanceTransforms.Add(UKismetMathLibrary::MakeRelativeTransform(FTransform(
		FRotator::ZeroRotator,
		FVector(0.f, SecondaryY, SecondaryZ),
		FVector::OneVector), BaseTransform));
	
	InstanceTransforms.Add(UKismetMathLibrary::MakeRelativeTransform(FTransform(
	FRotator(0.f,0.f, -90.f),
	FVector(0.f, SecondaryY * -1.f, SecondaryZ),
	FVector::OneVector), BaseTransform));
	
	PanelCorner->AddInstances(InstanceTransforms, false, false, true);

	// Sides
	ScaleSafeInstance(PanelSide, UKismetMathLibrary::MakeRelativeTransform(
		FTransform(FRotator(0.f,0.f, 90.f),
			FVector(0.f, ScaledLocationY, ScaledLocationZ),
			FVector(1.f, PrimaryScaleZ, 1.f)), BaseTransform));
	
	ScaleSafeInstance(PanelSide, UKismetMathLibrary::MakeRelativeTransform(
		FTransform(FRotator(0.f,0.f, -90.f),
			FVector(0.f, ScaledLocationY * -1.f, ScaledLocationZ),
			FVector(1.f, PrimaryScaleZ, 1.f)), BaseTransform));
	
	ScaleSafeInstance(PanelSide, UKismetMathLibrary::MakeRelativeTransform(
		FTransform(FRotator::ZeroRotator,
			FVector(0.f,0.f, (Length - 2.f) * 100.f),
			FVector(1.f, PrimaryScaleY, 1.f)), BaseTransform));
}

void FNSamplesDisplayBuilder::ScaleSafeInstance(UInstancedStaticMeshComponent* Instance, const FTransform& Transform)
{
	if (const FVector Scale = Transform.GetScale3D();
		Scale.X > 0.f && Scale.Y > 0.f && Scale.Z > 0.f)
	{
		Instance->AddInstance(Transform);
	}
}

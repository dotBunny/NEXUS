// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSamplesDisplayBuilder.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

void FNSamplesDisplayBuilder::CreateDisplayInstances(UInstancedStaticMeshComponent* PanelCurve, 
	UInstancedStaticMeshComponent* PanelCurveEdge, UInstancedStaticMeshComponent* PanelMain, 
	UInstancedStaticMeshComponent* PanelCorner, UInstancedStaticMeshComponent* PanelSide,
	FTransform& MainPanelTransform, FTransform& FloorPanelTransform, float Depth, float Width, float Height)
{
	// Ensure width
	if (Width < 2.f) Width = 2.f;
	
	if (Height >= 2.f && Depth >= 2.f)
	{
		const float WidthY = (Width - 2.f) * 0.5f;
		PanelCurve->AddInstance(
			FTransform(
				FRotator::ZeroRotator,
				FVector::ZeroVector,
				FVector(1.f, WidthY, 1.f)), false);

		TArray<FTransform> InstanceTransforms;
		InstanceTransforms.Add(FTransform(
					
			FRotator::ZeroRotator,
			FVector(0.f, WidthY * 100.f, 0.f),
			FVector::OneVector));
		
		InstanceTransforms.Add(FTransform(
		FRotator(90.f, 360.f, 180.f),
		FVector(0.f, WidthY * -100.f, 0.f),
		FVector::OneVector));
		PanelCurveEdge->AddInstances(InstanceTransforms, false, false, true);
	}
	
	// Panels
	if (Depth >= 2.f)
	{
		if (Height >= 2.f)
		{
			// Traditional Display
			MainPanelTransform = FTransform(FRotator::ZeroRotator, FVector(0.f, 0.f, -100.f), FVector::OneVector);
			FNSamplesDisplayBuilder::CreateScalablePanelInstances(MainPanelTransform,PanelMain, PanelCorner, PanelSide, Height, Width);
			FloorPanelTransform = FTransform(FRotator(90.f, 180.f, 0.f), FVector(0.f, 0.f, -100.f), FVector::OneVector);
			FNSamplesDisplayBuilder::CreateScalablePanelInstances(FloorPanelTransform, PanelMain, PanelCorner, PanelSide,Depth, Width);
		}
		else
		{
			// Floor
			MainPanelTransform = FTransform(FRotator(-90.f, 0.f, 0.f), FVector::ZeroVector, FVector::OneVector);
			FNSamplesDisplayBuilder::CreateScalablePanelInstances(MainPanelTransform,PanelMain, PanelCorner, PanelSide, 1.f, Width, true);
			FloorPanelTransform = FTransform(FRotator(90.f, 180.f, 0.f), FVector(0.f, 0.f, -100), FVector::OneVector); 
			FNSamplesDisplayBuilder::CreateScalablePanelInstances(FloorPanelTransform, PanelMain, PanelCorner, PanelSide,Depth, Width);
		}
	}
	else
	{

		if (Height < 2.f)
		{
			Height = 2.f;
		}
		FloorPanelTransform = FTransform(FRotator::ZeroRotator, FVector(0.f, 0.f, ((Height * 0.5f)) - 102.f), FVector::OneVector);
		FNSamplesDisplayBuilder::CreateScalablePanelInstances(FloorPanelTransform, PanelMain, PanelCorner, PanelSide,Height, Width);
		MainPanelTransform = FTransform(FRotator(180.f, 180.f, 0.f), FVector(0.f, 0.f, 100.f), FVector::OneVector);
		FNSamplesDisplayBuilder::CreateScalablePanelInstances(MainPanelTransform,PanelMain, PanelCorner, PanelSide, 2.f, Width, false);
	}
}

void FNSamplesDisplayBuilder::CreateScalablePanelInstances(const FTransform& BaseTransform, 
	UInstancedStaticMeshComponent* PanelMain, UInstancedStaticMeshComponent* PanelCorner, 
	UInstancedStaticMeshComponent* PanelSide, const float Depth, const float Width, const bool bIgnoreMainPanel)
{
	const float PrimaryScaleY = (Width - 2) * 0.5f;
	const float PrimaryScaleZ = (Depth - 2) * 0.5f;
	const float SecondaryY = ((Width - 2.f) * .5f) * 100.f;
	const float SecondaryZ = (Depth - 2.f) * 100.f;
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
			FVector(0.f,0.f, (Depth - 2.f) * 100.f),
			FVector(1.f, PrimaryScaleY, 1.f)), BaseTransform));
}

void FNSamplesDisplayBuilder::CreateShadowBoxInstances(UInstancedStaticMeshComponent* ShadowBoxSide, 
	UInstancedStaticMeshComponent* ShadowBoxTop, UInstancedStaticMeshComponent* ShadowBoxRound, 
	float ShadowBoxCoverDepthPercentage, float Depth, float Width, float Height)
{
	// Ensure the cover is only there when we need it and it will behave appropriately
	if (!(ShadowBoxCoverDepthPercentage > 0.05f && Height > 2.0f) || Depth <= 1.f) return;

	TArray<FTransform> InstanceTransforms;
	
	InstanceTransforms.Add(
		FTransform(FRotator::ZeroRotator,
			FVector(0.f, Width * 50.f, 0.f),
			FVector((Depth - 1.f) * ShadowBoxCoverDepthPercentage, 1.f, (Height - 1.f) / 5.f)));
	InstanceTransforms.Add(
			FTransform(FRotator(0.f, 0.f, 180.f),
			(InstanceTransforms[0].GetLocation() * -1.f) + FVector(0.f, 0.f, (Height * 100) - 100.f),
			InstanceTransforms[0].GetScale3D()));
	
	ShadowBoxSide->AddInstances(InstanceTransforms, false, false, true);
	InstanceTransforms.Empty();
	
	ShadowBoxTop->AddInstance(FTransform(
		FRotator(0.f, 0.f, -90.f),
		FVector(0.f, (Width - 2.0f) * 50.f, Height * 100.f),
		FVector((Depth - 1.f) * ShadowBoxCoverDepthPercentage, 1.f, (Width - 2.f) / 5.f)));
	
	InstanceTransforms.Add(FTransform(
		FRotator::ZeroRotator,
		FVector(0.f, Width * 50.f, (Height - 1.0) * 100.f),
		FVector((Depth - 1.f) * ShadowBoxCoverDepthPercentage, 1.f, 1.f)));
	
	const FVector Location = InstanceTransforms[0].GetLocation();
	InstanceTransforms.Add(FTransform(
		FRotator(0.f, 0.f, -90.f),
		FVector(Location.X, (Location.Y * -1.f) + 100.f, Location.Z + 100.f),
		InstanceTransforms[0].GetScale3D()));
	
	ShadowBoxRound->AddInstances(InstanceTransforms, false, false, true);
}

void FNSamplesDisplayBuilder::CreateTitlePanelInstances(UInstancedStaticMeshComponent* TitleBarMain, 
	UInstancedStaticMeshComponent* TitleBarEndLeft, UInstancedStaticMeshComponent* TitleBarEndRight,
	const float Depth, const float Width)
{
	// Ensure width doesn't make it look bad
	float WidthLimited = Width;
	if (WidthLimited < 3.f)
	{
		WidthLimited = 3;
	}
	
	const FTransform BaseTransform = FTransform(
		FRotator::ZeroRotator,
		FVector((Depth - 1.f) * 100.f, 0.f, 0.f),
		FVector(1.f, WidthLimited - 3.f, 1.f));
	
	TitleBarMain->AddInstance(BaseTransform, false);
	const FVector BaseLocation = BaseTransform.GetLocation();
	
	TitleBarEndLeft->AddInstance(FTransform(
			FRotator::ZeroRotator,
			FVector(BaseLocation.X, BaseLocation.Y + (((WidthLimited - 3.f) * 50.f) * -1.f), 
				BaseLocation.Z),
			FVector::OneVector), false);
	
	TitleBarEndRight->AddInstance(FTransform(
		FRotator::ZeroRotator,
		FVector(BaseLocation.X, BaseLocation.Y + ((WidthLimited - 3.f) * 50.f), BaseLocation.Z),
		FVector::OneVector), false); 
}

void FNSamplesDisplayBuilder::ApplyDefaultInstanceStaticMeshSettings(UInstancedStaticMeshComponent* Instance, 
	USceneComponent* PartRoot)
{
	Instance->SetupAttachment(PartRoot);
	Instance->SetMobility(EComponentMobility::Static);
	Instance->SetCollisionProfileName(FName("NoCollision"));
}

void FNSamplesDisplayBuilder::ScaleSafeInstance(UInstancedStaticMeshComponent* Instance, const FTransform& Transform)
{
	if (const FVector Scale = Transform.GetScale3D();
		Scale.X > 0.f && Scale.Y > 0.f && Scale.Z > 0.f)
	{
		Instance->AddInstance(Transform);
	}
}

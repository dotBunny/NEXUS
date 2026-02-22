// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Visualizers/NActorPoolSpawnerComponentVisualizer.h"

#include "NToolingEditorUserSettings.h"
#include "SceneManagement.h"
#include "NActorPoolSpawnerComponent.h"

void FNActorPoolSpawnerComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UNActorPoolSpawnerComponent* SpawnerComponent = Cast<const UNActorPoolSpawnerComponent>(Component);
	if (SpawnerComponent == nullptr)
	{
		return;
	}

	const FVector Origin = SpawnerComponent->GetComponentLocation() + SpawnerComponent->GetOffset();
	const FRotator Rotator = SpawnerComponent->GetComponentRotation();
	const UNToolingEditorUserSettings* Settings = UNToolingEditorUserSettings::Get();
	
	switch (SpawnerComponent->GetDistribution())
	{
		using enum ENActorPoolSpawnerDistribution;
		case Point:
		{
			DrawCircle(PDI, Origin,
				FRotationMatrix(Rotator).GetScaledAxis(EAxis::X), FRotationMatrix(Rotator).GetScaledAxis(EAxis::Y),
				Settings->VisualizationDistributionInnerColor, 5.f, Settings->VisualizationCircleSides, SDPG_Foreground, Settings->VisualizationLineThickness);
			break;
		}
		case Radius:
		{
			const FVector RadiusRange = SpawnerComponent->GetDistributionRange();
			DrawCircle(PDI, Origin,
				FRotationMatrix(Rotator).GetScaledAxis(EAxis::X), FRotationMatrix(Rotator).GetScaledAxis(EAxis::Y),
				Settings->VisualizationDistributionInnerColor, RadiusRange.X, Settings->VisualizationCircleSides, SDPG_Foreground, Settings->VisualizationLineThickness);
			DrawCircle(PDI, Origin,
				FRotationMatrix(Rotator).GetScaledAxis(EAxis::X), FRotationMatrix(Rotator).GetScaledAxis(EAxis::Y),
				Settings->VisualizationDistributionOuterColor, RadiusRange.Y, Settings->VisualizationCircleSides, SDPG_Foreground, Settings->VisualizationLineThickness);
			break;
		}
		case Sphere:
		{
			const FVector SphereRange = SpawnerComponent->GetDistributionRange();
			const FTransform& SphereTransform = SpawnerComponent->GetComponentTransform();
			DrawWireSphere(PDI, SphereTransform, Settings->VisualizationDistributionInnerColor, SphereRange.X, Settings->VisualizationCircleSides, SDPG_Foreground, Settings->VisualizationLineThickness);
			DrawWireSphere(PDI, SphereTransform, Settings->VisualizationDistributionOuterColor, SphereRange.Y, Settings->VisualizationCircleSides, SDPG_Foreground, Settings->VisualizationLineThickness);
			break;
		}
		case Box:
		{
			const FVector BoxRange = SpawnerComponent->GetDistributionRange();
			const FBox ABox = FBox(Origin - BoxRange, Origin + BoxRange);
			DrawWireBox(PDI, ABox, Settings->VisualizationDistributionOuterColor, SDPG_Foreground, Settings->VisualizationLineThickness);
			break;
		}
		case OrientedBox:
		{
			FTransform VLogTransform(SpawnerComponent->GetDistributionRotation(), SpawnerComponent->GetComponentTransform().GetLocation());
			FMatrix Matrix = VLogTransform.ToMatrixWithScale();
			const FVector BoxRange = SpawnerComponent->GetDistributionRange() * 0.5f;
			const FBox OBox = FBox(-BoxRange, BoxRange);
			DrawWireBox(PDI, Matrix, OBox, Settings->VisualizationDistributionOuterColor, SDPG_Foreground, Settings->VisualizationLineThickness);
			break;
		}
		case Spline:
		{
			break;
		}
	}
}


// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Visualizers/NActorPoolSpawnerComponentVisualizer.h"
#include "SceneManagement.h"
#include "NActorPoolSpawnerComponent.h"
#include "NEditorUserSettings.h"

void FNActorPoolSpawnerComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UNActorPoolSpawnerComponent* SpawnerComponent = Cast<const UNActorPoolSpawnerComponent>(Component);
	if (SpawnerComponent == nullptr)
	{
		return;
	}

	const FVector Origin = SpawnerComponent->GetComponentLocation() + SpawnerComponent->GetOffset();
	const FRotator Rotator = SpawnerComponent->GetComponentRotation();
	const UNEditorUserSettings* Settings = UNEditorUserSettings::Get();

	constexpr FColor DistributionOuterColor = FColor(200,0,0);
	constexpr FColor DistributionInnerColor = FColor(100,0,0);
	
	switch (SpawnerComponent->GetDistribution())
	{
		case APSD_Point:
		{
			DrawCircle(PDI, Origin,
				FRotationMatrix(Rotator).GetScaledAxis(EAxis::X), FRotationMatrix(Rotator).GetScaledAxis(EAxis::Y),
				DistributionOuterColor, 5.f, Settings->DebugCircleSides, SDPG_Foreground, Settings->DebugLineThickness);
			break;
		}
		
		case APSD_Radius:
		{
			const FVector RadiusRange = SpawnerComponent->GetDistributionRange();
			DrawCircle(PDI, Origin,
				FRotationMatrix(Rotator).GetScaledAxis(EAxis::X), FRotationMatrix(Rotator).GetScaledAxis(EAxis::Y),
				DistributionInnerColor, RadiusRange.X, Settings->DebugCircleSides, SDPG_Foreground, Settings->DebugLineThickness);
			DrawCircle(PDI, Origin,
				FRotationMatrix(Rotator).GetScaledAxis(EAxis::X), FRotationMatrix(Rotator).GetScaledAxis(EAxis::Y),
				DistributionOuterColor, RadiusRange.Y, Settings->DebugCircleSides, SDPG_Foreground, Settings->DebugLineThickness);
			break;
		}

		case APSD_Sphere:
		{
			const FVector SphereRange = SpawnerComponent->GetDistributionRange();
			const FTransform& SphereTransform = SpawnerComponent->GetComponentTransform();
			DrawWireSphere(PDI, SphereTransform, DistributionInnerColor, SphereRange.X, Settings->DebugCircleSides, SDPG_Foreground, Settings->DebugLineThickness);
			DrawWireSphere(PDI, SphereTransform, DistributionOuterColor, SphereRange.Y, Settings->DebugCircleSides, SDPG_Foreground, Settings->DebugLineThickness);
			break;
		}

		case APSD_Box:
		{
			const FVector BoxRange = SpawnerComponent->GetDistributionRange();
			const FBox Box = FBox(Origin - BoxRange, Origin + BoxRange);
			DrawWireBox(PDI, Box, DistributionOuterColor, SDPG_Foreground, Settings->DebugLineThickness);
			break;
		}

		case APSD_Spline:
		{
			break;
		}
	}
}


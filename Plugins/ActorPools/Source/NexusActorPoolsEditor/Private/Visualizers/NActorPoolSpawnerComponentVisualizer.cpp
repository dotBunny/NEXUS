// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Visualizers/NActorPoolSpawnerComponentVisualizer.h"

#include "NActorPoolsEditorUserSettings.h"
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
	const UNActorPoolsEditorUserSettings* Settings = UNActorPoolsEditorUserSettings::Get();
	
	switch (SpawnerComponent->GetDistribution())
	{
		using enum ENActorPoolSpawnerDistribution;
		case Point:
		{
			DrawCircle(PDI, Origin,
				FRotationMatrix(Rotator).GetScaledAxis(EAxis::X), FRotationMatrix(Rotator).GetScaledAxis(EAxis::Y),
				Settings->DistributionOuterColor, 5.f, Settings->DebugCircleSides, SDPG_Foreground, Settings->DebugLineThickness);
			break;
		}
		case Radius:
		{
			const FVector RadiusRange = SpawnerComponent->GetDistributionRange();
			DrawCircle(PDI, Origin,
				FRotationMatrix(Rotator).GetScaledAxis(EAxis::X), FRotationMatrix(Rotator).GetScaledAxis(EAxis::Y),
				Settings->DistributionInnerColor, RadiusRange.X, Settings->DebugCircleSides, SDPG_Foreground, Settings->DebugLineThickness);
			DrawCircle(PDI, Origin,
				FRotationMatrix(Rotator).GetScaledAxis(EAxis::X), FRotationMatrix(Rotator).GetScaledAxis(EAxis::Y),
				Settings->DistributionOuterColor, RadiusRange.Y, Settings->DebugCircleSides, SDPG_Foreground, Settings->DebugLineThickness);
			break;
		}
		case Sphere:
		{
			const FVector SphereRange = SpawnerComponent->GetDistributionRange();
			const FTransform& SphereTransform = SpawnerComponent->GetComponentTransform();
			DrawWireSphere(PDI, SphereTransform, Settings->DistributionInnerColor, SphereRange.X, Settings->DebugCircleSides, SDPG_Foreground, Settings->DebugLineThickness);
			DrawWireSphere(PDI, SphereTransform, Settings->DistributionOuterColor, SphereRange.Y, Settings->DebugCircleSides, SDPG_Foreground, Settings->DebugLineThickness);
			break;
		}
		case Box:
		{
			const FVector BoxRange = SpawnerComponent->GetDistributionRange();
			const FBox ABox = FBox(Origin - BoxRange, Origin + BoxRange);
			DrawWireBox(PDI, ABox, Settings->DistributionOuterColor, SDPG_Foreground, Settings->DebugLineThickness);
			break;
		}
		case OrientedBox:
		{
			FTransform VLogTransform(SpawnerComponent->GetDistributionRotation(), SpawnerComponent->GetComponentTransform().GetLocation());
			FMatrix Matrix = VLogTransform.ToMatrixWithScale();
			const FVector BoxRange = SpawnerComponent->GetDistributionRange() * 0.5f;
			const FBox OBox = FBox(-BoxRange, BoxRange);
			DrawWireBox(PDI, Matrix, OBox, Settings->DistributionOuterColor, SDPG_Foreground, Settings->DebugLineThickness);
			break;
		}
		case Spline:
		{
			break;
		}
	}
}


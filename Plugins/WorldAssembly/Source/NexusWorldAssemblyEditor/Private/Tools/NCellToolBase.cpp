// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Tools/NCellToolBase.h"

#include "Cell/NCellRootComponent.h"

bool UNCellToolBuilderBase::CanBuildTool(const FToolBuilderState& SceneState) const
{
	return UNWorldAssemblyEdMode::HasCellActor();
}

void UNCellToolBase::Setup()
{
	UInteractiveTool::Setup();

	CellActor = UNWorldAssemblyEdMode::GetCellActor();

	// The overlays these tools edit are drawn by the mode and the cell-root visualizer, both of which switch on the
	// mode's cell-edit sub-mode. Setting it here is what makes starting a tool show the geometry it operates on.
	if (const TOptional<UNWorldAssemblyEdMode::ENCellEdMode> EdMode = GetCellEdMode(); EdMode.IsSet())
	{
		UNWorldAssemblyEdMode::SetCellEdMode(EdMode.GetValue());
	}
}

void UNCellToolBase::Shutdown(EToolShutdownType ShutdownType)
{
	CellActor.Reset();

	UInteractiveTool::Shutdown(ShutdownType);
}

UNCellRootComponent* UNCellToolBase::GetCellRoot() const
{
	ANCellActor* Actor = CellActor.Get();
	return Actor != nullptr ? Actor->GetCellRoot() : nullptr;
}

void UNCellToolBase::MarkCellDirty() const
{
	if (ANCellActor* Actor = CellActor.Get())
	{
		Actor->SetActorDirty();
	}
}

bool UNCellToolBase::IsPickedByRay(const FRay& Ray, const FVector& Point, double& OutRayParameter)
{
	/** World-space pick radius per unit of distance from the ray origin. */
	constexpr double PickRadiusPerUnitDistance = 0.012;

	OutRayParameter = FMath::Max(0.0, FVector::DotProduct(Point - Ray.Origin, Ray.Direction));

	const double DistanceToAxis = FVector::Distance(Point, Ray.Origin + Ray.Direction * OutRayParameter);
	return DistanceToAxis <= OutRayParameter * PickRadiusPerUnitDistance;
}

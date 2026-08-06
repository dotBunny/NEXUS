// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Visualizers/NCellRootComponentVisualizer.h"

#include "Cell/NCellRootComponent.h"
#include "NWorldAssemblyEditorColors.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblySettings.h"

void FNCellRootComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	// First we check that we indeed have an actor component
	const UNCellRootComponent* CellRootComponent = Cast<UNCellRootComponent>(const_cast<UActorComponent*>(Component));
	if (!CellRootComponent)
	{
		return;
	}

	// We're not going to draw proxies, but its unlikely this is gonna be ever hit
	const ANCellActor* CellActor = CellRootComponent->GetNCellActor();
	if (CellActor != nullptr && CellActor->WasSpawnedFromProxy())
	{
		return;
	}


	// We need to draw the base wireframes
	if (!UNWorldAssemblyEdMode::IsActive())
	{
		CellRootComponent->DrawDebugPDI(PDI, static_cast<uint8>(UNWorldAssemblyEdMode::GetCellVoxelMode()),
			FNWorldAssemblyEditorColors::GetCellBounds(), FNWorldAssemblyEditorColors::GetCellHull());
		return;
	}

	// The overlays below are the points and lines the interactive tools act on, so they follow whichever cell-edit
	// sub-mode is active. No hit proxies any more: picking moved to the tools' own input behaviours when the edit mode
	// gained them, and leaving proxies here would put a second, competing click path over the same geometry.
	if (UNWorldAssemblyEdMode::GetCellEdMode() == UNWorldAssemblyEdMode::ENCellEdMode::Bounds)
	{
		const FBox Bounds = UNWorldAssemblyEdMode::GetCachedBounds();

		PDI->DrawPoint(Bounds.Min, FNWorldAssemblyEditorColors::GetCellBounds(), NEXUS::WorldAssembly::EdModeMetrics::HandleSize, SDPG_World);
		PDI->DrawPoint(Bounds.Max, FNWorldAssemblyEditorColors::GetCellBounds(), NEXUS::WorldAssembly::EdModeMetrics::HandleSize, SDPG_World);
	}
	else if (UNWorldAssemblyEdMode::GetCellEdMode() == UNWorldAssemblyEdMode::ENCellEdMode::Hull)
	{
		const TArray<FVector>& WorldVertices = UNWorldAssemblyEdMode::GetCachedHullVertices();
		for (const FVector& WorldVertex : WorldVertices)
		{
			PDI->DrawPoint(WorldVertex, FNWorldAssemblyEditorColors::GetCellHull(), NEXUS::WorldAssembly::EdModeMetrics::HandleSize, SDPG_World);
		}

		for (const TArray<FIntVector2>& WorldEdges = UNWorldAssemblyEdMode::GetCachedHullEdges();
			const FIntVector2& WorldEdge : WorldEdges)
		{
			if (!WorldVertices.IsValidIndex(WorldEdge.X) || !WorldVertices.IsValidIndex(WorldEdge.Y)) continue;

			PDI->DrawLine(WorldVertices[WorldEdge.X], WorldVertices[WorldEdge.Y],
				FNWorldAssemblyEditorColors::GetCellHull(), 2.f, SDPG_World);
		}
	}
	else if (UNWorldAssemblyEdMode::GetCellEdMode() == UNWorldAssemblyEdMode::ENCellEdMode::Voxel)
	{
		// Forcibly disable drawing of the voxel Mode
		if (UNWorldAssemblyEdMode::GetCellVoxelMode() != UNWorldAssemblyEdMode::ENCellVoxelMode::None)
		{
			UNWorldAssemblyEdMode::SetCellVoxelMode(UNWorldAssemblyEdMode::ENCellVoxelMode::None);
		}

		const FNCellVoxelData& CachedData = UNWorldAssemblyEdMode::GetCachedVoxelData();
		if (!CachedData.IsValid())
		{
			return;
		}

		const size_t PointCount = CachedData.GetCount();
		const UNWorldAssemblySettings* Settings = GetDefault<UNWorldAssemblySettings>();
		const FVector UnitSize = Settings->VoxelSize;
		const FVector HalfUnitSize = UnitSize * 0.5f;
		const FVector BaseOffset = CachedData.Origin;
		for (int32 i = 0; i < PointCount; i++)
		{
			auto [x,y,z] = CachedData.GetInverseIndex(i);

			// TODO: #ROTATE-VOXELS Rotation needs to actually rotated to the nearest grid???
			FVector VoxelCenter = BaseOffset + ((FVector(x, y, z) * UnitSize) + HalfUnitSize);

			const bool bOccupied = N_FLAGS_HAS(CachedData.GetData(i), static_cast<uint8>(ENCellVoxel::Occupied));
			PDI->DrawPoint(VoxelCenter, bOccupied ? FColor::Blue : FColor::Green, NEXUS::WorldAssembly::EdModeMetrics::HandleSize, SDPG_Foreground);
		}
	}
}
